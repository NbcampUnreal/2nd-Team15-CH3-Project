// Fill out your copyright notice in the Description page of Project Settings.

#include "Equipment/EquipmentManagerComponent.h"
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

	//if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	//{
	//	for (const TObjectPtr<const ULyraAbilitySet>& AbilitySet : EquipmentCDO->AbilitySetsToGrant)
	//	{
	//		AbilitySet->GiveToAbilitySystem(ASC, &NewEntry.GrantedHandles, Result);
	//	}
	//}
	//else
	//{
	//	
	//}

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
			//if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			//{
			//	Item.GrantedHandles.TakeFromAbilitySystem(ASC);
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


//UEquipmentInstance* UEquipmentManagerComponent::GetFirstInstanceOfType(TSubclassOf<UEquipmentInstance> InstanceType)
//{
//	
//}
//
//TArray<UEquipmentInstance*> UEquipmentManagerComponent::GetEquipmentInstancesOfType(
//	TSubclassOf<UEquipmentInstance> InstanceType) const
//{
//}
