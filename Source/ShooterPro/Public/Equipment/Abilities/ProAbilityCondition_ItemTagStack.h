// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProAbilityCondition.h"
#include "ProAbilityCondition_ItemTagStack.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProAbilityCondition_ItemTagStack : public UProAbilityCondition
{
	GENERATED_BODY()

	virtual bool CheckCondition(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo* ActorInfo,
								FGameplayTagContainer* OptionalRelevantTags) const override;
};
