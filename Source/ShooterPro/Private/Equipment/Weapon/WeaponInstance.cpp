// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Weapon/WeaponInstance.h"

TSubclassOf<UAnimInstance> FAnimSelectionSet::SelectLayer(const FGameplayTagContainer& CosmeticTags) const
{
	return nullptr;
}

UWeaponInstance::UWeaponInstance(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

void UWeaponInstance::OnEquipped()
{
	Super::OnEquipped();
}

void UWeaponInstance::OnUnequipped()
{
	Super::OnUnequipped();
}

void UWeaponInstance::UpdateFiringTime()
{
}

float UWeaponInstance::GetTimeSinceLastInteractedWith() const
{
	return 0.0f;
}
