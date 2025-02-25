// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProGameplayAbility_EquipmentBase.h"

#include "Equipment/EquipmentInstance.h"

UEquipmentInstance* UProGameplayAbility_EquipmentBase::GetSourceEquipmentInstance()
{
	if (FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec())
	{
		return Cast<UEquipmentInstance>(Spec->SourceObject.Get());
	}

	return nullptr;
}
