// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProGameplayAbility_EquipmentBase.h"

#include "AbilitySystem/Abilities/ProAbilityCost.h"
#include "Equipment/EquipmentInstance.h"
#include "Inventory/InventoryItemInstance.h"

UEquipmentInstance* UProGameplayAbility_EquipmentBase::GetSourceEquipmentInstance() const
{
	if (FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UEquipmentInstance>(Spec->SourceObject.Get());
	}

	return nullptr;
}

UInventoryItemInstance* UProGameplayAbility_EquipmentBase::GetSourceItemInstance() const
{
	if (UEquipmentInstance* Equipment = GetSourceEquipmentInstance())
	{
		return Cast<UInventoryItemInstance>(Equipment->GetInstigator());
	}
	return nullptr;
}
