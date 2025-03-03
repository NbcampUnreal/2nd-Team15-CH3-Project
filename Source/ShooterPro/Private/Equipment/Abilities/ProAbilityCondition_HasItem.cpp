// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProAbilityCondition_HasItem.h"

#include "Inventory/InventoryManagerComponent.h"

bool UProAbilityCondition_HasItem::CheckCondition(const UProGameplayAbility* Ability,
                                                  const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                  FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		// 아바타가 없으면 조건 불충족
		return false;
	}

	// AvatarActor에서 UAIBehaviorsComponent를 찾아서 AttackTarget 확인
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	UInventoryManagerComponent* InventoryManager= AvatarActor->FindComponentByClass<UInventoryManagerComponent>();
	if (!InventoryManager)	return false;
	
	return InventoryManager->HasEnoughItem(RequiredItem, StackCount);
}
