// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Equipment/Weapon/WeaponInstance.h"
#include "GameplayTags.h"
#include "RangedWeaponInstance.generated.h"


UCLASS()
class SHOOTERPRO_API URangedWeaponInstance : public UWeaponInstance
{
	GENERATED_BODY()
public:
	URangedWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void Tick(float DeltaSecond);
	
	int32 GetBulletsPerCartridge() const
	{
		return BulletsPerCartridge;
	}
	
	float GetCalculatedSpreadAngle() const
	{
		return CurrentSpreadAngle;
	}

	float GetCalculatedSpreadAngleMultiplier() const
	{
		return bHasFirstShotAccuracy ? 0.0f : CurrentSpreadAngleMultiplier;
	}

	bool HasFirstShotAccuracy() const
	{
		return bHasFirstShotAccuracy;
	}

	float GetSpreadExponent() const
	{
		return SpreadExponent;
	}

	float GetBulletTraceSweepRadius() const
	{
		return BulletTraceSweepRadius;
	}

	float GetLineTraceRange() const
	{
		return LineTraceRange;
	}

	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;

protected:
	
	// 탄 퍼짐 계수(높을수록 정확도가 올라감.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin=0.1), Category="Spread|Fire Params")
	float SpreadExponent = 1.0f;

	// X축은 무기 과열, Y축은 탄 퍼짐 각도
	UPROPERTY(EditAnywhere, Category = "Spread|Fire Params")
	FRuntimeFloatCurve HeatToSpreadCurve;

	// A curve that maps the current heat to the amount a single shot will further 'heat up'
	// This is typically a flat curve with a single data point indicating how much heat a shot adds,
	// but can be other shapes to do things like punish overheating by adding progressively more heat.
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	FRuntimeFloatCurve HeatToHeatPerShotCurve;
	
	// X축은 현재 온도, Y축은 냉각 속도
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	FRuntimeFloatCurve HeatToCoolDownPerSecondCurve;

	// 탄 퍼짐 감소를 위한(냉각) 딜레이 시간
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params", meta=(ForceUnits=s))
	float SpreadRecoveryCooldownDelay = 0.0f;

	// 첫 발은 무조건 정확도 100으로
	UPROPERTY(EditAnywhere, Category="Spread|Fire Params")
	bool bAllowFirstShotAccuracy = false;

	// 에이밍 중 탄 퍼짐 계수
	UPROPERTY(EditAnywhere, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_Aiming = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_StandingStill = 1.0f;

	// 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_StandingStill = 5.0f;

	// 정지 상태를 결정하는 계수 (어느 정도의 이동은 정지로 취급할거임)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits="cm/s"))
	float StandingStillSpeedThreshold = 80.0f;

	// 정지->이동 시 사용할 계수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits="cm/s"))
	float StandingStillToMovingSpeedRange = 20.0f;

	// 앉아있을 때 탄 퍼짐 계수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_Crouching = 1.0f;

	// 숙이거나 일어날 때 계수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_Crouching = 5.0f;
	
	// 공중에서의 계수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params", meta=(ForceUnits=x))
	float SpreadAngleMultiplier_JumpingOrFalling = 1.0f;

	// 공중과 지상 상태 변경 중 계수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Spread|Player Params")
	float TransitionRate_JumpingOrFalling = 5.0f;

	// 한 번 쏠 때 나갈 총알의 갯수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config")
	int32 BulletsPerCartridge = 1;

	// 탄이 좀 더 곡선으로 나가게 만들게
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon Config", meta=(ForceUnits=cm))
	float BulletTraceSweepRadius = 0.0f;
	
	// 총 쏘기 전 라인트레이싱에 사용할 end 거리
	UPROPERTY(EditAnywhere, Category = "Weapon Config")
	float LineTraceRange;
private:
	void ComputeSpreadRange(float& MinSpread, float& MaxSpread);
	void ComputeHeatRange(float& MinHeat, float& MaxHeat);

	inline float ClampHeat(float NewHeat)
	{
		float MinHeat;
		float MaxHeat;
		ComputeHeatRange( MinHeat, MaxHeat);

		return FMath::Clamp(NewHeat, MinHeat, MaxHeat);
	}
	
	bool UpdateSpread(float DeltaSecond);
	bool UpdateMultipliers(float DeltaSeconds);
	
	double LastFireTime = 0.0;

	float CurrentHeat = 0.0f;

	float CurrentSpreadAngle = 0.0f;

	bool bHasFirstShotAccuracy = false;

	float CurrentSpreadAngleMultiplier = 1.0f;

	float StandingStillMultiplier = 1.0f;

	float JumpFallMultiplier = 1.0f;

	float CrouchingMultiplier = 1.0f;	
};
