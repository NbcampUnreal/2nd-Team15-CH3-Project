// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProAbilityCost_HasEnoughItem.h"

#include "Inventory/InventoryManagerComponent.h"

bool UProAbilityCost_HasEnoughItem::CheckCost(const UProGameplayAbility* Ability,
                                              const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                              FGameplayTagContainer* OptionalRelevantTags) const
{
	//인벤토리 매니저 가져와서 아이템이 충분한지 확인
	if (!ensure(ActorInfo)) return false;

	if (ActorInfo->AvatarActor == nullptr) return false;

	UInventoryManagerComponent* InventoryManager = Cast<UInventoryManagerComponent>(ActorInfo->AvatarActor->GetComponentByClass(UInventoryManagerComponent::StaticClass()));

	if (!ensure(InventoryManager)) return false;
	
	return InventoryManager->HasEnoughItem(ItemDef, Cost);
}

void UProAbilityCost_HasEnoughItem::ApplyCost(const UProGameplayAbility* Ability,
	const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	// 아이템 갯수 줄이기
	if (!ensure(ActorInfo)) return;

	if (ActorInfo->AvatarActor == nullptr) return;

	UInventoryManagerComponent* InventoryManager = Cast<UInventoryManagerComponent>(ActorInfo->AvatarActor->GetComponentByClass(UInventoryManagerComponent::StaticClass()));

	if (!ensure(InventoryManager)) return;

	InventoryManager->ConsumeItemsByDefinition(ItemDef, Cost);
}
