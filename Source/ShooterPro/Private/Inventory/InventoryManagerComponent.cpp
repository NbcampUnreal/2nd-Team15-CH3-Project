// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryManagerComponent.h"
#include "Inventory/InventoryItemInstance.h"
#include "Inventory/InventoryItemDefinition.h"

UInventoryItemInstance* FInventoryList::AddItem(const TSubclassOf<UInventoryItemDefinition>& ItemDef, int32 StackCount)
{
	UInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
	check(OwnerComponent);
	
	if (Items.Contains(ItemDef))
	{
		Items[ItemDef].StackCount += StackCount;
	}
	else
	{
		FInventoryItem NewItem;
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
		Items.Add(ItemDef, NewItem);
		Result = NewItem.Instance;
	}
	
	return Result;
}

bool FInventoryList::RemoveItem(const TSubclassOf<UInventoryItemDefinition>& ItemDef, int32 StackCount)
{
	if (StackCount <= 0) return false;

	if (!Items.Contains(ItemDef)) return false;

	FInventoryItem& Item = Items[ItemDef];
	
	if (Item.StackCount < StackCount) return false;

	Item.StackCount -= StackCount;

	if (Item.StackCount == 0) 
	{
		Items.Remove(ItemDef);
	}
	
	return true;
}

void FInventoryList::EraseItem(UInventoryItemInstance* Instance)
{
	Items.Remove(Instance->GetItemDef());
}

TArray<UInventoryItemInstance*> FInventoryList::GetAllItems() const
{
	TArray<UInventoryItemInstance*> Results;
	Results.Reserve(Items.Num());

	for (const auto& Entry : Items)
	{
		if (Entry.Value.Instance != nullptr) Results.Add(Entry.Value.Instance);
	}

	return Results;
}

FInventoryItem FInventoryList::FindFirstItemWithDefinition(const TSubclassOf<UInventoryItemDefinition>& ItemDef) const
{
	return Items.Contains(ItemDef) ? Items[ItemDef] : FInventoryItem();
}


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

void UInventoryManagerComponent::EraseItemInstance(UInventoryItemInstance* ItemInstance)
{
	InventoryList.EraseItem(ItemInstance);
}

UInventoryItemInstance* UInventoryManagerComponent::FindFirstItemInstanceByDefinition(
	TSubclassOf<UInventoryItemDefinition> ItemDef) const
{
	TArray<UInventoryItemInstance*> Items = InventoryList.GetAllItems();
	
	for (UInventoryItemInstance* Item : Items)
	{
		if (Item->GetItemDef() == ItemDef) return Item;
	}

	return nullptr;
}

FInventoryItem UInventoryManagerComponent::FindFirstInventoryItemByDefinition(
	TSubclassOf<UInventoryItemDefinition> ItemDef) const
{
	
	return InventoryList.FindFirstItemWithDefinition(ItemDef);
}

bool UInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef,
                                                          int32 NumToConsume)
{
	return InventoryList.RemoveItem(ItemDef, NumToConsume);
}
