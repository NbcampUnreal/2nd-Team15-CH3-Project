
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileAOEActor.generated.h"

class UGameplayEffect;
class USphereComponent;

UCLASS()
class SHOOTERPRO_API AProjectileAOEActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProjectileAOEActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// AoE 종료
	void DestroyAOE();

protected:
	UPROPERTY(VisibleAnywhere, Category="Projectile AOE|Collision")
	USceneComponent* SceneComponent;
	
	UPROPERTY(VisibleAnywhere, Category="Projectile AOE|Collision")
	USphereComponent* AOETriggerSphere;

	// AoE 범위 안에 들어오는 캐릭터에게 줄 Debuff
	UPROPERTY(EditDefaultsOnly, Category="Projectile AOE|AOE")
	TSubclassOf<UGameplayEffect> AOEEffectClass;

	// AoE 유지 시간
	UPROPERTY(EditDefaultsOnly, Category="Projectile AOE|AOE")
	float AoeDuration;

	FTimerHandle AoeTimerHandle;
};
