// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "ProjectileManagerComponent.generated.h"

class AProBulletBase;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProjectileManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UProjectileManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	
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
