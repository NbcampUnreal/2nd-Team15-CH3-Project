// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inventory/InventoryItemDefinition.h"
#include "Styling/SlateBrush.h"
#include "InventoryFragment_QuickBarIcon.generated.h"

UCLASS()
class SHOOTERPRO_API UInventoryFragment_QuickBarIcon : public UInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FText DisplayNameWhenEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	TSubclassOf<UInventoryItemDefinition> AmmoType;
};
