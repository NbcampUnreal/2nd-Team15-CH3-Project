#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AICollisionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollisionDisabled);

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollisionEnabled, ECollisionPart, CollisionPart);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, FHitResult, HitResult);


UCLASS(ClassGroup=("AI"), meta=(BlueprintSpawnableComponent))
class SHOOTERPRO_API UAICollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAICollisionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
//
// public:
// 	UFUNCTION(BlueprintCallable, Category="Collision")
// 	void EnableCollision(ECollisionPart CollisionPart);
//
// 	UFUNCTION(BlueprintCallable, Category="Collision")
// 	void DisableCollision();
//
// 	UFUNCTION(BlueprintCallable, Category="Collision")
// 	void SetCollisions(const TArray<FHitCollisionComponent>& NewHitCollisions);
//
// 	UFUNCTION(BlueprintCallable, Category="Collision")
// 	void AddHitActor(UPrimitiveComponent* Component, AActor* HitActor);
//
// 	UFUNCTION(BlueprintCallable, Category="Collision")
// 	void SpawnHitVFXAndSound(const FHitResult& HitResult, UParticleSystem* HitVFX, USoundBase* HitSound, FVector Scale = FVector(1.0f));
//
// 	UFUNCTION(BlueprintCallable, Category="Collision")
// 	void ApplyDamageToActor(const FHitResult& Hit, float BaseDamage);
//
// 	UFUNCTION(BlueprintPure, Category="Collision")
// 	bool IsCollisionActive() const { return bTraceActive; }
//
// protected:
// 	UFUNCTION()
// 	void ActivateTrace();
//
// 	UFUNCTION()
// 	void UpdateLastSocketPositions();
//
// public:
// 	UFUNCTION(BlueprintPure, Category="Cillision")
// 	FName GetUniqueSocketName(UPrimitiveComponent* Component, FName SocketName);
//
// 	UFUNCTION(BlueprintCallable, Category="Cillision")
// 	int32 GetHitActorsIndex(UPrimitiveComponent* Component);
//
// 	UFUNCTION(BlueprintPure, Category="Cillision")
// 	TArray<AActor*> GetHitActors(UPrimitiveComponent* Component);
//
//
// 	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PROPERTIES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// public:
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Config")
// 	float TraceRadius = 0.5f;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Config")
// 	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Config")
// 	float DrawTime = 5.0f;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Config")
// 	TEnumAsByte<EDrawDebugTrace::Type> DrawDebug;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Collision|Config")
// 	TArray<FName> IgnoredActorTag;
//
// 	UPROPERTY(BlueprintReadWrite, Category="Collision")
// 	bool bTraceActive = false;
//
// 	UPROPERTY(BlueprintReadWrite, Category="Collision")
// 	bool bCollisionActive = false;
//
// 	UPROPERTY(BlueprintReadOnly, Category="Collision")
// 	TArray<FHitCollisionComponent> CollisionComponents;
//
// 	UPROPERTY(BlueprintReadOnly, Category="Collision")
// 	TArray<FHitActorCollisionComponent> HitActors;
//
// 	UPROPERTY(BlueprintReadOnly, Category="Collision")
// 	TMap<FName, FVector> LastSocketPositions;
//
// public:
// 	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Collision|Delegate")
// 	FOnCollisionEnabled OnCollisionEnabled;
//
// 	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Collision|Delegate")
// 	FOnCollisionDisabled OnCollisionDisabled;
//
// 	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Collision|Delegate")
// 	FOnHit OnHit;
};
