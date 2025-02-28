// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/ProGSCAbilitySet.h"

#include "GSCLog.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "Abilities/GSCAbilitySystemUtils.h"


bool UProGSCAbilitySet::GrantToAbilitySystemWithSource(UAbilitySystemComponent* InASC, UObject* SourceObject,
                                                       FGSCAbilitySetHandle& OutAbilitySetHandle, FText* OutErrorText, const bool bShouldRegisterCoreDelegates) const
{
	if (!IsValid(InASC)) return false;

	UGSCAbilitySystemComponent* ASC = Cast<UGSCAbilitySystemComponent>(InASC);
	if (!ASC) return false;

	const bool bSuccess = TryGrantAbilitySetWithSource(ASC, this, SourceObject, OutAbilitySetHandle);

	// Make sure to re-register delegates, this set may have added
	if (bSuccess && bShouldRegisterCoreDelegates)
	{
		TryRegisterCoreComponentDelegates(InASC);
	}

	return bSuccess;
}

bool UProGSCAbilitySet::TryGrantAbilitySetWithSource(UAbilitySystemComponent* InASC, const UGSCAbilitySet* InAbilitySet,
	UObject* SourceObject, FGSCAbilitySetHandle& OutAbilitySetHandle, TArray<TSharedPtr<FComponentRequestHandle>>* OutComponentRequests)
{
	check(InASC);
	
	if (!InAbilitySet)
	{
		return false;
	}

	{
		using namespace UE::GASCompanion::Log;
		const FString WorldPrefix = GetWorldLogPrefix(InASC->GetWorld());
		GSC_PLOG(
			Verbose,
			TEXT("%s: IsOwnerActorAuthoritative: %s, OuterOwner: %s, Role: %s, HasAuthority: %s"),
			*WorldPrefix,
			*GetBoolText(InASC->IsOwnerActorAuthoritative()),
			*GetNameSafe(InASC->GetOwner()),
			*UEnum::GetValueAsString<ENetRole>(InASC->GetOwnerRole()),
			*GetBoolText(InASC->GetOwner()->HasAuthority())
		);
	}

	// Add Abilities
	int32 AbilitiesIndex = 0;
	for (const FGSCGameFeatureAbilityMapping& AbilityMapping : InAbilitySet->GrantedAbilities)
	{
		AbilitiesIndex++;
		
		if (AbilityMapping.AbilityType.IsNull())
		{
			GSC_PLOG(Error, TEXT("GrantedAbilities AbilityType on ability set %s is not valid at Index %d"), *GetNameSafe(InAbilitySet), AbilitiesIndex  - 1);
			continue;
		}

		// Try to grant the ability first
		FGameplayAbilitySpec AbilitySpec;
		FGameplayAbilitySpecHandle AbilityHandle;
		TryGrantAbility(InASC, AbilityMapping, AbilityHandle, AbilitySpec, SourceObject);
		OutAbilitySetHandle.Abilities.Add(AbilityHandle);

		// Handle Input Mapping now
		if (!AbilityMapping.InputAction.IsNull())
		{
			FDelegateHandle DelegateHandle;
			FGSCAbilitySystemUtils::TryBindAbilityInput(InASC, AbilityMapping, AbilityHandle, AbilitySpec, DelegateHandle, OutComponentRequests);
			OutAbilitySetHandle.InputBindingDelegateHandles.Add(MoveTemp(DelegateHandle));
		}
	}

	// Add Attributes
	int32 AttributesIndex = 0;
	for (const FGSCGameFeatureAttributeSetMapping& Attributes : InAbilitySet->GrantedAttributes)
	{
		AttributesIndex++;

		if (Attributes.AttributeSet.IsNull())
		{
			GSC_PLOG(Error, TEXT("GrantedAttributes AttributeSet on ability set %s is not valid at Index %d"), *GetNameSafe(InAbilitySet), AttributesIndex - 1);
			continue;
		}

		UAttributeSet* AddedAttributeSet = nullptr;
		FGSCAbilitySystemUtils::TryGrantAttributes(InASC, Attributes, AddedAttributeSet);

		if (AddedAttributeSet)
		{
			OutAbilitySetHandle.Attributes.Add(AddedAttributeSet);
		}
	}

	// Add Effects
	int32 EffectsIndex = 0;
	for (const FGSCGameFeatureGameplayEffectMapping& Effect : InAbilitySet->GrantedEffects)
	{
		EffectsIndex++;
		
		if (Effect.EffectType.IsNull())
		{
			GSC_PLOG(Error, TEXT("GrantedEffects EffectType on ability set %s is not valid at Index %d"), *GetNameSafe(InAbilitySet), EffectsIndex - 1);
			continue;
		}

		FGSCAbilitySystemUtils::TryGrantGameplayEffect(InASC, Effect.EffectType.LoadSynchronous(), Effect.Level, OutAbilitySetHandle.EffectHandles);
	}

	// Add Owned Gameplay Tags
	if (InAbilitySet->OwnedTags.IsValid())
	{		
		FGSCAbilitySystemUtils::AddLooseGameplayTagsUnique(InASC, InAbilitySet->OwnedTags);

		// Store a copy of the tags, so that they can be removed later on from handle
		OutAbilitySetHandle.OwnedTags = InAbilitySet->OwnedTags;
	}
	
	// Store the name of the Ability Set "instigator"
	OutAbilitySetHandle.AbilitySetPathName = InAbilitySet->GetPathName();
	return true;
}

void UProGSCAbilitySet::TryGrantAbility(UAbilitySystemComponent* InASC,
	const FGSCGameFeatureAbilityMapping& InAbilityMapping, FGameplayAbilitySpecHandle& OutAbilityHandle,
	FGameplayAbilitySpec& OutAbilitySpec, UObject* SourceObject)
{
	check(InASC);
	
	if (InAbilityMapping.AbilityType.IsNull())
	{
		GSC_PLOG(Error, TEXT("Failed to Grant Ability \"%s\" because SoftClassPtr is null"), *InAbilityMapping.AbilityType.ToString())
		return;
	}
	
	const TSubclassOf<UGameplayAbility> AbilityType = InAbilityMapping.AbilityType.LoadSynchronous();
	check(AbilityType);

	UGSCAbilitySystemComponent* ASC = Cast<UGSCAbilitySystemComponent>(InASC);
	if (!ASC)
	{
		GSC_PLOG(Error, TEXT("Failed to Grant Ability \"%s\" because ASC \"%s\" is not a UGSCAbilitySystemComponent"), *GetNameSafe(AbilityType), *GetNameSafe(InASC))
		return;
	}

	OutAbilitySpec = ASC->BuildAbilitySpecFromClass(AbilityType, InAbilityMapping.Level);
	OutAbilitySpec.SourceObject = SourceObject;
	// Try to grant the ability first
	if (ASC->IsOwnerActorAuthoritative())
	{
		// Only Grant abilities on authority, and only if we should (ability not granted yet or wants reset on spawn)
		if (!FGSCAbilitySystemUtils::IsAbilityGranted(ASC, AbilityType, InAbilityMapping.Level))
		{
			GSC_PLOG(Verbose, TEXT("Authority, Grant Ability (%s)"), *AbilityType->GetName())
			OutAbilityHandle = ASC->GiveAbility(OutAbilitySpec);
		}
		else
		{
			// In case granting is prevented because of ability already existing, return the existing handle
			const FGameplayAbilitySpec* ExistingAbilitySpec = ASC->FindAbilitySpecFromClass(AbilityType);
			if (ExistingAbilitySpec)
			{
				OutAbilityHandle = ExistingAbilitySpec->Handle;
			}
		}
	}
	else
	{
		// For clients, try to get ability spec and update handle used later on for input binding
		const FGameplayAbilitySpec* ExistingAbilitySpec = ASC->FindAbilitySpecFromClass(AbilityType);
		if (ExistingAbilitySpec)
		{
			OutAbilityHandle = ExistingAbilitySpec->Handle;
		}
		
		GSC_LOG(Verbose, TEXT("AddActorAbilities: Not Authority, try to find ability handle from spec: %s"), *OutAbilityHandle.ToString())
	}
}
