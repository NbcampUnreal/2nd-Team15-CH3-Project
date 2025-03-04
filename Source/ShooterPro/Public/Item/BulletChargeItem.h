// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletChargeItem.generated.h"

class USphereComponent;
class UInventoryItemDefinition;

UCLASS()
class SHOOTERPRO_API ABulletChargeItem : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletChargeItem();

protected:
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

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Component")
	TObjectPtr<USceneComponent> Scene;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Component")
	TObjectPtr<USphereComponent> ActivationCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item|Component")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	TObjectPtr<UParticleSystem> PickUpParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Effects")
	TObjectPtr<USoundBase> PickUpSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TSubclassOf<UInventoryItemDefinition> ItemToCharge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 StackToCharge;

};
