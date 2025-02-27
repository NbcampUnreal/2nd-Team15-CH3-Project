// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProAbilityCost.h"
#include "ProAbilityCost_TagStack.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProAbilityCost_TagStack : public UProAbilityCost
{
	GENERATED_BODY()

public:

	virtual bool CheckCost(
		const UProGameplayAbility* Ability,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		FGameplayTagContainer* OptionalRelevantTags
		) const override;

	// 어빌리티의 코스트를 적용하는 부분
	virtual void ApplyCost(
		const UProGameplayAbility* Ability,
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
		) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	int32 Cost;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	FGameplayTag Tag;
};
