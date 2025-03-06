// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryItemDefinition.h"
#include "ItemSpawnRow.generated.h"

USTRUCT(BlueprintType)
struct FItemSpawnRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UInventoryItemDefinition> SpawnDefinition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BulletAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnChance;
};