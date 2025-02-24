// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryItemDefinition.h"
#include "InventoryFragment_SetStats.generated.h"

class UObject;
struct FGameplayTag;

UCLASS()
class SHOOTERPRO_API UInventoryFragment_SetStats : public UInventoryItemFragment
{
	GENERATED_BODY()
protected:
	//UPROPERTY(EditDefaultsOnly, Category=Equipment)
	//TMap<FGameplayTag, int32> InitialItemStats;

public:
	virtual void OnInstanceCreated(UInventoryItemInstance* Instance) const override;

	//int32 GetItemStatByTag(FGameplayTag Tag) const;
};
