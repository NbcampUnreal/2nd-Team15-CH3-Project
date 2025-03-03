// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProAbilityCondition.h"
#include "ProAbilityCondition_HasItem.generated.h"
class UInventoryItemDefinition;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProAbilityCondition_HasItem : public UProAbilityCondition
{
	GENERATED_BODY()
public:
	virtual bool CheckCondition(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo* ActorInfo,
								FGameplayTagContainer* OptionalRelevantTags) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UInventoryItemDefinition> RequiredItem;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 StackCount;
};
