// Fill out your copyright notice in the Description page of Project Settings.

#include "Equipment/EquipmentManagerComponent.h"

#include "AbilitySystemGlobals.h"
#include "Abilities/GSCAbilitySet.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "AbilitySystem/ProGSCAbilitySet.h"
#include "Equipment/EquipmentDefinition.h"
#include "Equipment/EquipmentInstance.h"


UEquipmentManagerComponent::UEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), EquipmentList(this)
{
	bWantsInitializeComponent = true;
}

void UEquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UEquipmentManagerComponent::UninitializeComponent()
{
	TArray<UEquipmentInstance*> AllEquipmentInstances;

	// gathering all instances before removal to avoid side effects affecting the equipment list iterator	
	for (const FEquipmentItem& Item : EquipmentList.Items)
	{
		AllEquipmentInstances.Add(Item.Instance);
	}

	for (UEquipmentInstance* EquipInstance : AllEquipmentInstances)
	{
		UnequipItem(EquipInstance);
	}

	Super::UninitializeComponent();
}

UGSCAbilitySystemComponent* FEquipmentList::GetGSCAbilitySystemComponent() const
{
	check(OwnerComponent);
	AActor* OwnerActor = OwnerComponent->GetOwner();
	return Cast<UGSCAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerActor));
}

UEquipmentInstance* FEquipmentList::AddItem(TSubclassOf<UEquipmentDefinition> EquipmentDefinition)
{
	UEquipmentInstance* Result = nullptr;
	check(EquipmentDefinition != nullptr);
	check(OwnerComponent);

	const UEquipmentDefinition* EquipmentCDO = GetDefault<UEquipmentDefinition>(EquipmentDefinition);

	TSubclassOf<UEquipmentInstance> InstanceType = EquipmentCDO->InstanceType;

	if (InstanceType == nullptr)
	{
		InstanceType = UEquipmentInstance::StaticClass();
	}

	FEquipmentItem& NewEntry = Items.AddDefaulted_GetRef();
	NewEntry.EquipmentDefinition = EquipmentDefinition;
	NewEntry.Instance = NewObject<UEquipmentInstance>(OwnerComponent->GetOwner(), InstanceType); //@TODO: Using the actor instead of component as the outer due to UE-127172
	Result = NewEntry.Instance;

	if (UGSCAbilitySystemComponent* GASC = GetGSCAbilitySystemComponent())
	{
		for (const TObjectPtr<const UProGSCAbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
		{
			AbilitySet->GrantToAbilitySystemWithSource(GASC, Result, NewEntry.GrantedHandles);
		}
		
		//for (int32 i = 0; i < EquipmentCDO->AbilitySetsToGrant.Num(); i++)
		//{
		//	const TObjectPtr<const UProGSCAbilitySet>& AbilitySet = EquipmentCDO->AbilitySetsToGrant[i];
		//	AbilitySet->GrantToAbilitySystemWithSource(GASC, Result, NewEntry.GrantedHandles);
		//}
	}

	Result->SpawnEquipmentActors(EquipmentCDO->ActorsToSpawn);

	return Result;
}

void FEquipmentList::RemoveItem(UEquipmentInstance* Instance)
{
	for (auto Iter = Items.CreateIterator(); Iter; ++Iter)
	{
		FEquipmentItem& Item = *Iter;
		if (Item.Instance == Instance)
		{
			//if (UGSCAbilitySystemComponent* GASC = GetGSCAbilitySystemComponent())
			//{
			//	
			//	//Item.GrantedHandles.(GASC);
			//}

			Instance->DestroyEquipmentActors();

			Iter.RemoveCurrent();
		}
	}
}


UEquipmentInstance* UEquipmentManagerComponent::EquipItem(TSubclassOf<UEquipmentDefinition> EquipmentDefinition)
{
	UEquipmentInstance* Result = nullptr;
	if (EquipmentDefinition != nullptr)
	{
		Result = EquipmentList.AddItem(EquipmentDefinition);
		Result->OnInstanceCreated();

		if (Result != nullptr)
		{
			Result->OnEquipped();
		}
	}
	return Result;
}

void UEquipmentManagerComponent::UnequipItem(UEquipmentInstance* ItemInstance)
{
	if (ItemInstance != nullptr)
	{
		ItemInstance->OnUnequipped();
		EquipmentList.RemoveItem(ItemInstance);
	}
}


UEquipmentInstance* UEquipmentManagerComponent::GetFirstInstanceOfType(TSubclassOf<UEquipmentInstance> InstanceType)
{
	for (FEquipmentItem& Entry : EquipmentList.Items)
	{
		if (UEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

TArray<UEquipmentInstance*> UEquipmentManagerComponent::GetEquipmentInstancesOfType(
	TSubclassOf<UEquipmentInstance> InstanceType) const
{
	TArray<UEquipmentInstance*> Results;
	for (const FEquipmentItem& Entry : EquipmentList.Items)
	{
		if (UEquipmentInstance* Instance = Entry.Instance)
		{
			if (Instance->IsA(InstanceType))
			{
				Results.Add(Instance);
			}
		}
	}
	return Results;
}

UEquipmentInstance* UEquipmentManagerComponent::GetEquipmentInstanceByDefinition(TSubclassOf<UEquipmentDefinition> InDefinition) const
{
	// 간단히, 리스트를 순회하여 EquipmentDefinition이 동일한 엔트리를 찾음
	for (const FEquipmentItem& Entry : EquipmentList.Items)
	{
		if (Entry.EquipmentDefinition == InDefinition)
		{
			return Entry.Instance;
		}
	}
	return nullptr;
}
