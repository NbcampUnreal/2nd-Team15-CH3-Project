// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "ProGameplayAbility.h"
#include "UObject/NoExportTypes.h"
#include "ProAbilityCost.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew, Abstract)
class SHOOTERPRO_API UProAbilityCost : public UObject
{
	GENERATED_BODY()
public:
	UProAbilityCost() {}

	// 코스트를 적용 가능한지 확인하는 부분
	virtual bool CheckCost(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
	{
		return K2_CheckCost(Ability, Handle, *ActorInfo, *OptionalRelevantTags);
	}

	// 어빌리티의 코스트를 적용하는 부분
	virtual void ApplyCost(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
	{
		K2_ApplyCost(Ability, Handle, *ActorInfo, ActivationInfo);
	}
	
	UFUNCTION(BlueprintImplementableEvent)
	bool K2_CheckCost(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilityActorInfo& ActorInfo, FGameplayTagContainer& OptionalRelevantTags) const;

	UFUNCTION(BlueprintImplementableEvent)
	void K2_ApplyCost(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo);
	
};
