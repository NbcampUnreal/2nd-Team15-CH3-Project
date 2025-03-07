#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayCueNotify_Burst.h"
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
	//UFUNCTION(BlueprintCallable)
	virtual void ActivateBullet(AActor* AvatarActor, const FVector& SpawnLocation, const FRotator& SpawnRotation, const FVector& Direction, const float Speed = 2000.0f);
	
	UFUNCTION(BlueprintCallable)
	void DeactivateBullet();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void K2_ActivateBullet(AActor* Avatar, const FVector& SpawnLocation, const FRotator& SpawnRotation, const FVector& Direction, const float Speed = 2000.0f);
	
	// 충돌 처리 함수
	UFUNCTION(BlueprintNativeEvent)
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	UFUNCTION(BlueprintNativeEvent)
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

protected:
	UPROPERTY()
	AActor* AvatarActor;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Projectile|Config", meta=(ExposeOnSpawn=true))
	TSubclassOf<UGameplayEffect> DamageEffect;
};
