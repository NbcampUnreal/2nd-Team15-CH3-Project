// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryItemDefinition.h"
#include "InventoryFragment_EquippableItem.generated.h"

class UEquipmentDefinition;

UCLASS()
class SHOOTERPRO_API UInventoryFragment_EquippableItem : public UInventoryItemFragment
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = Inventory)
	TSubclassOf<UEquipmentDefinition> EquipmentDefinition;
};
