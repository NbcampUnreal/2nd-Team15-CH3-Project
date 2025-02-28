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

void FInventoryList::RemoveItem(const TSubclassOf<UInventoryItemDefinition>& ItemDef, int32 StackCount)
{
	if (!HasEnoughItem(ItemDef, StackCount)) return;

	FInventoryItem& Item = Items[ItemDef];

	Item.StackCount -= StackCount;

	if (Item.StackCount == 0)
	{
		Items.Remove(ItemDef);
	}
}

void FInventoryList::EraseItem(UInventoryItemInstance* Instance)
{
	Items.Remove(Instance->GetItemDef());
}

bool FInventoryList::HasEnoughItem(const TSubclassOf<UInventoryItemDefinition>& ItemDef, int32 StackCount) const
{
	if (StackCount <= 0 || !Items.Contains(ItemDef))
	{
		return false;
	}

	return Items[ItemDef].StackCount >= StackCount;
}

int FInventoryList::GetStackCount(TSubclassOf<UInventoryItemDefinition> ItemDef)
{
	if (!ItemDef || !Items.Contains(ItemDef)) return 0;

	return Items[ItemDef].StackCount;
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

FInventoryItem FInventoryList::FindItemByDefinition(const TSubclassOf<UInventoryItemDefinition>& ItemDef) const
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

FInventoryItem UInventoryManagerComponent::FindInventoryItemByDefinition(
	TSubclassOf<UInventoryItemDefinition> ItemDef) const
{
	return InventoryList.FindItemByDefinition(ItemDef);
}

bool UInventoryManagerComponent::HasEnoughItem(const TSubclassOf<UInventoryItemDefinition>& ItemDef,
                                               int32 StackCount) const
{
	return InventoryList.HasEnoughItem(ItemDef, StackCount);
}

void UInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef,
                                                          int32 NumToConsume)
{
	InventoryList.RemoveItem(ItemDef, NumToConsume);
}

int32 UInventoryManagerComponent::GetItemStackCount(TSubclassOf<UInventoryItemDefinition> ItemDef)
{
	return InventoryList.GetStackCount(ItemDef);
}

void UInventoryManagerComponent::AddItemStackCount(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount)
{
	InventoryList.AddItem(ItemDef, StackCount);
}

void UInventoryManagerComponent::SubtractItemStackCount(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount)
{
	InventoryList.RemoveItem(ItemDef, StackCount);
}

UInventoryItemInstance* UInventoryManagerComponent::GetItemInstanceByIndex(int32 Index) const
{
	// 1) 전체 아이템 목록을 배열로 얻어온다. (TArray<UInventoryItemInstance*>)
	TArray<UInventoryItemInstance*> AllItems = InventoryList.GetAllItems();

	// 2) Index가 배열 범위 내인지 체크
	if (AllItems.IsValidIndex(Index))
	{
		return AllItems[Index];
	}

	// 범위를 벗어나면 nullptr 반환
	return nullptr;
}

bool UInventoryManagerComponent::DoesItemExistAtIndex(int32 Index) const
{
	// GetItemInstanceByIndex를 재활용하여 존재 여부만 체크
	return (GetItemInstanceByIndex(Index) != nullptr);
}

TArray<UInventoryItemInstance*> UInventoryManagerComponent::GetAllItems()
{
	return InventoryList.GetAllItems();
}
