// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProGameplayAbility.h"
#include "ProGameplayAbility_EquipmentBase.generated.h"

class UEquipmentInstance;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProGameplayAbility_EquipmentBase : public UProGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Ability")
	UEquipmentInstance* GetSourceEquipmentInstance();
};
