// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Weapon/RangedWeaponInstance.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

URangedWeaponInstance::URangedWeaponInstance(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	HeatToHeatPerShotCurve.EditorCurveData.AddKey(0.0f, 1.0f);
	HeatToCoolDownPerSecondCurve.EditorCurveData.AddKey(0.0f, 2.0f);
}

void URangedWeaponInstance::Tick(float DeltaSecond)
{
	APawn* Pawn = GetOwnerAsPawn();
	check(Pawn);

	// 탄퍼짐 구하기
	const bool bIsMinSpread = UpdateSpread(DeltaSecond);
	// 보정값
	const bool bIsMinMultipliers = UpdateMultipliers(DeltaSecond);
	
	bHasFirstShotAccuracy = bAllowFirstShotAccuracy && bIsMinMultipliers & bIsMinSpread;
}

void URangedWeaponInstance::OnEquipped()
{
	K2_OnEquipped();

	float MinHeatRange;
	float MaxHeatRange;

	ComputeHeatRange(MinHeatRange, MaxHeatRange);
	CurrentHeat = (MinHeatRange + MaxHeatRange) * 0.5f;

	CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurveConst()->Eval(CurrentHeat);

	CurrentSpreadAngleMultiplier = 1.0f;
	StandingStillMultiplier = 1.0f;
	JumpFallMultiplier = 1.0f;
	CrouchingMultiplier = 1.0f;
}

void URangedWeaponInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

void URangedWeaponInstance::ComputeSpreadRange(float& MinSpread, float& MaxSpread)
{
	HeatToSpreadCurve.GetRichCurveConst()->GetValueRange(/*out*/ MinSpread, /*out*/ MaxSpread);
}

void URangedWeaponInstance::ComputeHeatRange(float& MinHeat, float& MaxHeat)
{
	float Min1;
	float Max1;
	HeatToHeatPerShotCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min1, /*out*/ Max1);

	float Min2;
	float Max2;
	HeatToCoolDownPerSecondCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min2, /*out*/ Max2);

	float Min3;
	float Max3;
	HeatToSpreadCurve.GetRichCurveConst()->GetTimeRange(/*out*/ Min3, /*out*/ Max3);

	MinHeat = FMath::Min(FMath::Min(Min1, Min2), Min3);
	MaxHeat = FMath::Max(FMath::Max(Max1, Max2), Max3);
}

// 
bool URangedWeaponInstance::UpdateSpread(float DeltaSecond)
{
	const float TimeSinceFired = GetWorld()->TimeSince(LastFireTime); // 쏜 시간 간격

	if (TimeSinceFired > SpreadRecoveryCooldownDelay)
	{
		// 냉각속도 그래프에서 현재 열기에 따라 쿨다운 속도 결정
		const float CooldownRate = HeatToCoolDownPerSecondCurve.GetRichCurve()->Eval(CurrentHeat);
		// 현재 열 업데이트
		CurrentHeat = ClampHeat(CurrentHeat - (CooldownRate * DeltaSecond));
		// 탄 퍼짐 각도 업데이트 (이것도 커브에서 현재 열 상태를 받아서 결정)
		CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurve()->Eval(CurrentHeat);
	}

	float MinSpread;
	float MaxSpread;

	//
	ComputeSpreadRange(MinSpread, MaxSpread);
	
	// 퍼짐 각도가 MinSpread와 거의 다를게 없다면 true
	return FMath::IsNearlyEqual(CurrentSpreadAngle, MinSpread, 1.e-4f);
}

bool URangedWeaponInstance::UpdateMultipliers(float DeltaSeconds)
{
	const float MultiplierNearlyEqualThreshold = 0.05f;

	APawn* Pawn = GetOwnerAsPawn();
	check(Pawn != nullptr);
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());

	//이동 보정값
	const float PawnSpeed = Pawn->GetVelocity().Length();
	const float MovementValue = FMath::GetMappedRangeValueClamped(
		FVector2D(StandingStillSpeedThreshold, StandingStillSpeedThreshold + StandingStillToMovingSpeedRange),
		FVector2D(SpreadAngleMultiplier_StandingStill),
		PawnSpeed
	);
	StandingStillMultiplier = FMath::FInterpTo(StandingStillMultiplier, MovementValue, DeltaSeconds, TransitionRate_StandingStill);
	const bool bStandingStillMultiplierAtMin = FMath::IsNearlyEqual(StandingStillMultiplier, SpreadAngleMultiplier_StandingStill, SpreadAngleMultiplier_StandingStill*0.1f);

	// 숙이기 보정값
	const bool bIsCrouching = (CharMovementComp != nullptr) && CharMovementComp->IsCrouching();
	const float CrouchingTargetValue = bIsCrouching ? SpreadAngleMultiplier_Crouching : 1.0f;

	// 현재 값을 CrouchingTargetValue 값과 가깝게 만드는 것이 목표
	CrouchingMultiplier = FMath::FInterpTo(CrouchingMultiplier, CrouchingTargetValue, DeltaSeconds, TransitionRate_Crouching);
	const bool bCrouchingMultiplierAtTarget = FMath::IsNearlyEqual(CrouchingMultiplier, CrouchingTargetValue, MultiplierNearlyEqualThreshold);

	// 공중 보정값
	const bool bIsJumpingOrFalling = (CharMovementComp != nullptr) && CharMovementComp->IsFalling();
	const float JumpFallTargetValue = bIsJumpingOrFalling ? SpreadAngleMultiplier_JumpingOrFalling : 1.0f;
	JumpFallMultiplier = FMath::FInterpTo(JumpFallMultiplier, JumpFallTargetValue, DeltaSeconds, TransitionRate_JumpingOrFalling);
	const bool bJumpFallMultiplerIs1 = FMath::IsNearlyEqual(JumpFallMultiplier, 1.0f, MultiplierNearlyEqualThreshold);

	// ADS 보정값 -> 태그나 카메라 가져오기.
	float AimingAlpha = 0.0f;
	//if (const UCameraComponent* CameraComponent = (Pawn))
	//{
	//	float TopCameraWeight;
	//	FGameplayTag TopCameraTag;
	//	CameraComponent->GetBlendInfo(/*out*/ TopCameraWeight, /*out*/ TopCameraTag);

	//	AimingAlpha = (TopCameraTag == TAG_Lyra_Weapon_SteadyAimingCamera) ? TopCameraWeight : 0.0f;
	//}
	
	const float AimingMultiplier = FMath::GetMappedRangeValueClamped(
		/*InputRange=*/ FVector2D(0.0f, 1.0f),
		/*OutputRange=*/ FVector2D(1.0f, SpreadAngleMultiplier_Aiming),
		/*Alpha=*/ AimingAlpha);
	const bool bAimingMultiplierAtTarget = FMath::IsNearlyEqual(AimingMultiplier, SpreadAngleMultiplier_Aiming, KINDA_SMALL_NUMBER);

	// Combine all the multipliers
	const float CombinedMultiplier = AimingMultiplier * StandingStillMultiplier * CrouchingMultiplier * JumpFallMultiplier;
	CurrentSpreadAngleMultiplier = CombinedMultiplier;

	// 4가지 조건 (가만히 있는가, 앉아있는가)
	return bStandingStillMultiplierAtMin && bCrouchingMultiplierAtTarget && bJumpFallMultiplerIs1 && bAimingMultiplierAtTarget;
}
