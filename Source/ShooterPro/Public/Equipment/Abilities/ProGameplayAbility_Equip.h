// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Equipment/Abilities/ProGameplayAbility_EquipmentBase.h"
#include "ProGameplayAbility_Equip.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProGameplayAbility_Equip : public UProGameplayAbility_EquipmentBase
{
	GENERATED_BODY()

public:
	UProGameplayAbility_Equip();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
		) override;
	
	
};
