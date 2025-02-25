// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemInstance.h"
#include "BulletItemInstance.generated.h"

class AProBulletBase;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UBulletItemInstance : public UInventoryItemInstance
{
	GENERATED_BODY()

public:
	// 풀에서 탄환을 가져옴 (필요시 스폰)
	UFUNCTION(BlueprintCallable)
	AProBulletBase* RequestBullet(TSubclassOf<AProBulletBase> BulletClass);

	// 탄환을 풀로 되돌려놓음
	UFUNCTION(BlueprintCallable)
	void ReleaseBullet(AProBulletBase* Bullet);

public:
	UPROPERTY(EditDefaultsOnly, Category=Ammo)
	TSubclassOf<AProBulletBase> ProjectileActorClass;

protected:
	UPROPERTY()
	TArray<AProBulletBase*> BulletPool; // 이미 스폰된 탄환 목록
};
