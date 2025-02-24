// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/EquipmentInstance.h"
#include "WeaponInstance.generated.h"

struct FGameplayTagContainer;
class UAnimInstance;

USTRUCT(BlueprintType)
struct FAnimLayerSelection
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> Layer;
};

USTRUCT(BlueprintType)
struct FAnimSelectionSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAnimLayerSelection> LayerRules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> DefaultLayer;

	TSubclassOf<UAnimInstance> SelectLayer(const FGameplayTagContainer& CosmeticTags) const;
};

UCLASS()
class SHOOTERPRO_API UWeaponInstance : public UEquipmentInstance
{
	GENERATED_BODY()
public:
	UWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;

	UFUNCTION(BlueprintCallable)
	void UpdateFiringTime();

	UFUNCTION(BlueprintPure)
	float GetTimeSinceLastInteractedWith() const;

protected:
//Need AnimationSet
	
	double TimeLastEquipped = 0.;
	double TimeLastFired = 0.;
};
