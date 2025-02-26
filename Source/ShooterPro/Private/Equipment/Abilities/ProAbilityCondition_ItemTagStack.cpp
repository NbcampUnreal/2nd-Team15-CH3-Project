// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProAbilityCondition_ItemTagStack.h"

#include "Equipment/EquipmentManagerComponent.h"
#include "Equipment/Weapon/RangedWeaponInstance.h"
#include "Inventory/InventoryManagerComponent.h"

bool UProAbilityCondition_ItemTagStack::CheckCondition(const UProGameplayAbility* Ability,
                                                       const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                       FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid()) return false;
	
	// AvatarActor에서 UAIBehaviorsComponent를 찾아서 AttackTarget 확인
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();

	UInventoryManagerComponent* InventoryManager = Cast<UInventoryManagerComponent>(AvatarActor->GetComponentByClass(UInventoryManagerComponent::StaticClass()));

	if (!ensure(InventoryManager)) return false;
	
	return true;
}
