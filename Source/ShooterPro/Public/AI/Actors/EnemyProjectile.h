#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyProjectile.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class AProjectileAOEActor;
class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class SHOOTERPRO_API AEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	AEnemyProjectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	           FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

protected:
	// 1) 충돌 담당
	UPROPERTY(VisibleDefaultsOnly, Category="Enemy Projectile")
	USphereComponent* CollisionSphere;

	// 2) (필요없다면 제거) 메시
	UPROPERTY(VisibleDefaultsOnly, Category="Enemy Projectile")
	UStaticMeshComponent* ProjectileMesh;

	// 3) 이동 컴포넌트
	UPROPERTY(VisibleDefaultsOnly, Category="Enemy Projectile")
	UProjectileMovementComponent* ProjectileMovement;

	// 4) 날아가는 토사물 나이아가라 컴포넌트
	UPROPERTY(VisibleDefaultsOnly, Category="Enemy Projectile")
	UNiagaraComponent* ProjectileVomitNiagara;

public:
	// [벽/바닥 충돌 시] 생성할 나이아가라 시스템(잔류 토사물)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Enemy Projectile|Effects|Niagara")
	UNiagaraSystem* ImpactEffectWorld;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Enemy Projectile|Effects|Niagara")
	FVector ImpactScale = FVector(1.f);

	// [플레이어 피격 시] 나이아가라 시스템(혈흔)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Enemy Projectile|Effects|Niagara")
	UNiagaraSystem* ImpactEffectPlayer;

	// [날아가는 동안] 보여줄 나이아가라 시스템 (루프)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Enemy Projectile|Effects|Niagara")
	UNiagaraSystem* FlyingVomitNiagara;

	// [GAS] 플레이어 피격 시 적용할 디버프
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Enemy Projectile|Effects|Gameplay")
	TSubclassOf<UGameplayEffect> DeBuffEffectClass;

	// [벽/바닥 충돌 시] AoE 액터
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Enemy Projectile|Effects|Gameplay")
	TSubclassOf<AProjectileAOEActor> AOEActorClass;
};
