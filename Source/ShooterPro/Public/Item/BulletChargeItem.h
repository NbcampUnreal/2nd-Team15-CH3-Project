// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletChargeItem.generated.h"

class USphereComponent;
class UNiagaraSystem;
class UInventoryItemDefinition;
struct FItemSpawnRow;

UCLASS()
class SHOOTERPRO_API ABulletChargeItem : public AActor
{
	GENERATED_BODY()

public:
	ABulletChargeItem();

protected:
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnItemOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void ActivateItem(AActor* Activator);

	void DestroyItem();

	FItemSpawnRow* GetRandomItem() const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Component")
	TObjectPtr<USceneComponent> Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Component")
	TObjectPtr<USphereComponent> ActivationCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Component")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	TObjectPtr<UNiagaraSystem> PickUpNiagaraSystem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	TObjectPtr<USoundBase> PickUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Spawn")
	TObjectPtr<UDataTable> SpawnDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Animation")
	float RotationAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Animation")
	float BounceAmount;

	float TimeAdding;

};