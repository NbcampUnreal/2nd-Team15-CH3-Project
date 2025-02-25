// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProBulletBase.generated.h"

class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class SHOOTERPRO_API AProBulletBase : public AActor
{
	GENERATED_BODY()

public:
	AProBulletBase();

protected:
	virtual void BeginPlay() override;

public:
	// Called every frame
	// virtual void Tick(float DeltaTime) override;
public:
	UFUNCTION(BlueprintCallable)
	void ActivateBullet(const FVector& SpawnLocation, const FRotator& SpawnRotation, const FVector& InVelocity);
	
	UFUNCTION(BlueprintCallable)
	void DeactivateBullet();

protected:
	// 충돌 처리 함수
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

protected:
	// UPROPERTY(VisibleDefaultsOnly, Category="Projectile")
	// UStaticMeshComponent* BulletMesh;
	
	UPROPERTY(VisibleDefaultsOnly, Category="Projectile")
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category="Movement")
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Config", meta=(ExposeOnSpawn=true))
	FVector InitialDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Config", meta=(ExposeOnSpawn=true))
	float ImpactForceMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Config", meta=(ExposeOnSpawn=true))
	float GravityScale = 1.0f;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Config", meta=(ExposeOnSpawn=true))
	// float InitiailDeactivateBullet = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Config", meta=(ExposeOnSpawn=true))
	TSubclassOf<UGameplayEffect> DamageEffect;
};
