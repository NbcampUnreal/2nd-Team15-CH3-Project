// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/ProBulletBase.h"
#include "ProBulletGrenade.generated.h"

class UNiagaraComponent;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API AProBulletGrenade : public AProBulletBase
{
	GENERATED_BODY()

public:
	AProBulletGrenade();
	virtual void BeginPlay() override;
	//virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	virtual void ActivateBullet(AActor* AvatarActor, const FVector& SpawnLocation, const FRotator& SpawnRotation, const FVector& Direction, const float Speed = 2000.0f) override;
	
	//virtual void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
	UFUNCTION(BlueprintImplementableEvent)
	void Detonate();
	//virtual void Activate(AActor* AvatarActor, const FVector& SpawnLocation, const FRotator& SpawnRotation, const FVector& Direction, const float Speed = 2000.0f) override;
protected:

	void OnNiagaraEffectFinished(UNiagaraComponent* PSystem);
	UPROPERTY(BlueprintReadWrite, Category="Explode")
	float ExplodeTime = 3;
	UPROPERTY(BlueprintReadWrite, Category="Explode")
	FTimerHandle ExplodeTimer;
	UPROPERTY(BlueprintReadWrite, Category="Explode")
	float ExplodeRadius;
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explode")
	//USphereComponent* ExplosionSphere;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explode")
	//UNiagaraComponent* NiagaraComp;
};
