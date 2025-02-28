// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Weapon/RangedWeaponInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

URangedWeaponInstance::URangedWeaponInstance(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	HeatToHeatPerShotCurve.EditorCurveData.AddKey(0.0f, 1.0f);
	HeatToCoolDownPerSecondCurve.EditorCurveData.AddKey(0.0f, 2.0f);
}

void URangedWeaponInstance::Tick(float DeltaSecond)
{
	//APawn* Pawn = GetOwnerAsPawn();
	//check(Pawn);
	
	//const bool bMinSpread = UpdateSpread(DeltaSecond);
	//const bool bMinMultipliers = UpdateMultipliers(DeltaSecond);

	//bHasFirstShotAccuracy = bAllowFirstShotAccuracy && bMinMultipliers & bMinSpread;
}

void URangedWeaponInstance::OnEquipped()
{
	K2_OnEquipped();
}

void URangedWeaponInstance::OnUnequipped()
{
	K2_OnUnequipped();
}

void URangedWeaponInstance::AddSpread()
{
}

void URangedWeaponInstance::ComputeSpreadRange(float& MinSpread, float& MaxSpread)
{
}

void URangedWeaponInstance::ComputeHeatRange(float& MinHeat, float& MaxHeat)
{
}

bool URangedWeaponInstance::UpdateSpread(float DeltaSecond)
{
	const float TimeSinceFired = GetWorld()->TimeSince(LastFireTime); // LastFireTime Delay

	if (TimeSinceFired > SpreadRecoveryCooldownDelay)
	{
		const float CooldownRate = HeatToCoolDownPerSecondCurve.GetRichCurve()->Eval(CurrentHeat); //냉각속도 그래프
		CurrentHeat = ClampHeat(CurrentHeat - (CooldownRate * DeltaSecond));
		CurrentSpreadAngle = HeatToSpreadCurve.GetRichCurve()->Eval(CurrentHeat); //확산 각도 그래프
	}

	float MinSpread;
	float MaxSpread;

	ComputeSpreadRange(MinSpread, MaxSpread);
	return FMath::IsNearlyEqual(CurrentSpreadAngle, MinSpread, 1.e-4f);
}

bool URangedWeaponInstance::UpdateMultipliers(float DeltaSecond)
{
	const float MultiplierNearlyEqualThreshold = 0.05f;

	APawn* Pawn = GetOwnerAsPawn();
	check(Pawn != nullptr);
	UCharacterMovementComponent* CharMovementComp = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent());

	const float PawnSpeed = Pawn->GetVelocity().Length();
	const float MovementValue = FMath::GetMappedRangeValueClamped(
		FVector2D(StandingStillSpeedThreshold, StandingStillSpeedThreshold + StandingStillToMovingSpeedRange),
		FVector2D(SpreadAngleMultiplier_StandingStill),
		PawnSpeed
	);
	StandingStillMultiplier = FMath::FInterpTo(StandingStillMultiplier, MovementValue, DeltaSecond, TransitionRate_StandingStill);

	return false;
}
