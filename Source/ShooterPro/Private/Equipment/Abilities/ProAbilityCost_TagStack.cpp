// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProAbilityCost_TagStack.h"

#include "Equipment/QuickBarComponent.h"
#include "Equipment/Abilities/ProGameplayAbility_EquipmentBase.h"
#include "Inventory/InventoryItemInstance.h"

bool UProAbilityCost_TagStack::CheckCost(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	//어빌리티의 현재 액티브 아이템이 해당 태그가 Cost만큼 있는지 확인
	
	const UProGameplayAbility_EquipmentBase* EquipmentAbility = Cast<UProGameplayAbility_EquipmentBase>(Ability);

	if (!ensure(EquipmentAbility)) return false;
	
	UInventoryItemInstance* ItemInstance = EquipmentAbility->GetSourceItemInstance();
	
	if (!ensure(ItemInstance)) return false;
	
	return ItemInstance->GetStatTagStackCount(Tag) >= Cost;
}

void UProAbilityCost_TagStack::ApplyCost(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	//Cost만큼 Tag 삭제
	const UProGameplayAbility_EquipmentBase* EquipmentAbility = Cast<UProGameplayAbility_EquipmentBase>(Ability);

	if (!ensure(EquipmentAbility)) return;
	
	UInventoryItemInstance* ItemInstance = EquipmentAbility->GetSourceItemInstance();

	if (!ensure(ItemInstance)) return;

	ItemInstance->RemoveStatTagStack(Tag, Cost);
}
