// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Abilities/ProGameplayAbility_EquipmentBase.h"
#include "ProGameplayAbility_RangedWeapon.generated.h"

class AProBulletBase;
class URangedWeaponInstance;
class UProjectileManagerComponent;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProGameplayAbility_RangedWeapon : public UProGameplayAbility_EquipmentBase
{
	GENERATED_BODY()
public:

	//총알 스폰 (RangedWeaponInstance의 데이터로 변경 가능)
	UFUNCTION(BlueprintCallable)
	void FireWeapon(FVector StartLocation, FVector Direction, TSubclassOf<AProBulletBase> BulletClass);
	
	//레이케스팅 결과 찾는 함수
	UFUNCTION(BlueprintCallable)
	FVector GetHitResultWithRayCast(APlayerController* PlayerController);

	UFUNCTION(BlueprintCallable)
	FVector RandConeNormalDistribution(const FVector& Dir, const float ConeHalfDegree, const float Exponent);
	
	UFUNCTION(BlueprintCallable, Category="RangedWeapon")
	URangedWeaponInstance* GetSourceRangedWeaponInstance() const;
};
