// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryManagerComponent.h"
#include "Inventory/InventoryItemInstance.h"
#include "Inventory/InventoryItemDefinition.h"

UInventoryItemInstance* FInventoryList::AddItem(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());

	FInventoryItem& NewItem = Items.AddDefaulted_GetRef();
	NewItem.Instance = NewObject<UInventoryItemInstance>(OwnerComponent->GetOwner()); //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewItem.Instance->SetItemDef(ItemDef);
	for (UInventoryItemFragment* Fragment : GetDefault<UInventoryItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewItem.Instance);
		}
	}
	NewItem.StackCount = StackCount;
	Result = NewItem.Instance;

	return Result;
}

void FInventoryList::RemoveItem(UInventoryItemInstance* Instance)
{
}

TArray<UInventoryItemInstance*> FInventoryList::GetAllItems() const
{
	TArray<UInventoryItemInstance*> Results;
	Results.Reserve(Items.Num());

	for (const FInventoryItem& Entry : Items)
	{
		if (Entry.Instance != nullptr) Results.Add(Entry.Instance);
	}

	return Results;
}

// Sets default values for this component's properties
UInventoryManagerComponent::UInventoryManagerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer), InventoryList(this)
{
}

UInventoryItemInstance* UInventoryManagerComponent::AddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StakcCount)
{
	UInventoryItemInstance* Result = nullptr;

	if (ItemDef != nullptr)
	{
		Result = InventoryList.AddItem(ItemDef, StakcCount);
	}

	return Result;
}

void UInventoryManagerComponent::RemoveItemInstance(UInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveItem(ItemInstance);
}
