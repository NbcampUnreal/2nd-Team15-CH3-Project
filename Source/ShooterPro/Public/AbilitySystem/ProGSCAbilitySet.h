// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GSCAbilitySet.h"
#include "ProGSCAbilitySet.generated.h"

struct FComponentRequestHandle;

UCLASS()
class SHOOTERPRO_API UProGSCAbilitySet : public UGSCAbilitySet
{
	GENERATED_BODY()
public:
	bool GrantToAbilitySystemWithSource(UAbilitySystemComponent* InASC, UObject* SourceObject, FGSCAbilitySetHandle& OutAbilitySetHandle, FText* OutErrorText = nullptr, const bool bShouldRegisterCoreDelegates = true) const;
	static bool TryGrantAbilitySetWithSource(UAbilitySystemComponent* InASC, const UGSCAbilitySet* InAbilitySet, UObject* SourceObject, FGSCAbilitySetHandle& OutAbilitySetHandle, TArray<TSharedPtr<FComponentRequestHandle>>* OutComponentRequests = nullptr);
	static void TryGrantAbility(UAbilitySystemComponent* InASC, const FGSCGameFeatureAbilityMapping& InAbilityMapping, FGameplayAbilitySpecHandle& OutAbilityHandle, FGameplayAbilitySpec& OutAbilitySpec, UObject* SourceObject);
};
