// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProGameplayAbility_Equip.h"

#include "Character/ProCharacterBase.h"
#include "Equipment/EquipmentInstance.h"
#include "Equipment/QuickBarComponent.h"

UProGameplayAbility_Equip::UProGameplayAbility_Equip()
{
}

void UProGameplayAbility_Equip::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!ensure(AvatarActor))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	//UQuickBarComponent* QuickBar = AvatarActor->FindComponentByClass<UQuickBarComponent>();

	//if (!ensure(QuickBar))
	//{
	//	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	//	return;
	//}
//
	//UEquipmentManagerComponent* EquipmentManager = AvatarActor->GetComponentByClass<UEquipmentManagerComponent>();
//
	//if (!ensure(EquipmentManager))
	//{
	//	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	//	return;
	//}
	
	//QuickBar->GetActiveSlotItem();
	
	//UEquipmentInstance* EquipmentInstance = GetSourceEquipmentInstance();

	//if (!ensure(EquipmentInstance))
	//{
	//	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	//	return;
	//}
	
	//if (EquipmentInstance->EquippedAnimMontage)
	//{
	//}
}
