// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProAbilityCost.h"
#include "ProAbilityCost_HasEnoughItem.generated.h"

class UInventoryItemDefinition;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProAbilityCost_HasEnoughItem : public UProAbilityCost
{
	GENERATED_BODY()

public:
	virtual bool CheckCost(
		const UProGameplayAbility* Ability,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags
		) const override;
	
	virtual void ApplyCost(
		const UProGameplayAbility* Ability,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
		) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost")
	TSubclassOf<UInventoryItemDefinition> ItemDef;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Cost")
	int32 Cost;
};
