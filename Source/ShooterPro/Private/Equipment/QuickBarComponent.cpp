#include "Equipment/QuickBarComponent.h"

#include "ProGmaeplayTag.h"
#include "Equipment/EquipmentManagerComponent.h"
#include "Equipment/Weapon/RangedWeaponInstance.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/InventoryFragment_EquippableItem.h"
#include "Inventory/InventoryItemInstance.h"

UQuickBarComponent::UQuickBarComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.bCanEverTick = true;
}

void UQuickBarComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UQuickBarComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ActiveSlotIndex >= 0 && ActiveSlotIndex <= 2)
	{
		EquippedItem->Tick(DeltaTime);
	}
}

void UQuickBarComponent::CheckSlots()
{
	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}
}

void UQuickBarComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 2)
		return;

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex);

	int32 NewIndex = ActiveSlotIndex;

	do
	{
		NewIndex = (NewIndex + 1) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	}
	while (NewIndex != OldIndex);
}

void UQuickBarComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 2)
		return;

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	}
	while (NewIndex != OldIndex);
}

void UQuickBarComponent::ChangeQuickBarSlot(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex))
	{
		SetActiveSlotIndex(NewIndex);
	}
}

void UQuickBarComponent::SetActiveSlotIndex(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex) && (ActiveSlotIndex != NewIndex))
	{
		const int32 OldIndex = ActiveSlotIndex;

		UnequipItemInSlot();

		ActiveSlotIndex = NewIndex;

		EquipItemInSlot();

		if (OnSlotChanged.IsBound())
		{
			OnSlotChanged.Broadcast(OldIndex, NewIndex);
		}

		// 메시지 보낼 때, FQuickBarSlotData를 채운다.
		FQuickBarSlotData Payload = MakeSlotData(NewIndex);
		UGameplayMessageSubsystem& Subsystem = UGameplayMessageSubsystem::Get(this);
		Subsystem.BroadcastMessage<FQuickBarSlotData>(ProGameplayTags::Event_QuickBar_ActiveIndexChanged, Payload);
	}
}

UInventoryItemInstance* UQuickBarComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

int32 UQuickBarComponent::GetNextFreeItemSlot() const
{
	// ...
	int32 SlotIndex = 0;
	for (const TObjectPtr<UInventoryItemInstance>& ItemPtr : Slots)
	{
		if (ItemPtr == nullptr)
		{
			return SlotIndex;
		}
		++SlotIndex;
	}

	return INDEX_NONE;
}

UInventoryItemInstance* UQuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	// ...
	UInventoryItemInstance* Result = nullptr;

	if (ActiveSlotIndex == SlotIndex)
	{
		UnequipItemInSlot();
		ActiveSlotIndex = -1;
	}

	if (Slots.IsValidIndex(SlotIndex))
	{
		Result = Slots[SlotIndex];
		if (Result != nullptr)
		{
			Slots[SlotIndex] = nullptr;
			FQuickBarSlotData Payload = MakeSlotData(SlotIndex);

			UGameplayMessageSubsystem& Subsystem = UGameplayMessageSubsystem::Get(this);
			Subsystem.BroadcastMessage<FQuickBarSlotData>(ProGameplayTags::Event_QuickBar_SlotsRemovedChanged, Payload);
		}
	}
	return Result;
}

void UQuickBarComponent::AddItemToSlot(int32 SlotIndex, UInventoryItemInstance* Item)
{
	// ...
	CheckSlots();

	if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
	{
		if (Slots[SlotIndex] == nullptr)
		{
			Slots[SlotIndex] = Item;

			FQuickBarSlotData Payload = MakeSlotData(SlotIndex);
			UGameplayMessageSubsystem& Subsystem = UGameplayMessageSubsystem::Get(this);
			Subsystem.BroadcastMessage<FQuickBarSlotData>(ProGameplayTags::Event_QuickBar_SlotsAddedChanged, Payload);
		}
	}
}

void UQuickBarComponent::EquipItemInSlot()
{
	if (!Slots.IsValidIndex(ActiveSlotIndex))
		return;

	check(EquippedItem == nullptr);

	if (UInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
	{
		if (const UInventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			TSubclassOf<UEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
			if (EquipDef != nullptr)
			{
				if (UEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
				{
					EquippedItem = EquipmentManager->EquipItem(EquipDef);
					if (EquippedItem != nullptr)
					{
						EquippedItem->SetInstigator(SlotItem);
					}
				}
			}
		}
	}
}

void UQuickBarComponent::UnequipItemInSlot()
{
	if (UEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
	{
		if (EquippedItem != nullptr)
		{
			EquipmentManager->UnequipItem(EquippedItem);
			EquippedItem = nullptr;
		}
	}
}

UEquipmentManagerComponent* UQuickBarComponent::FindEquipmentManager() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UEquipmentManagerComponent>();
	}
	return nullptr;
}

FQuickBarSlotData UQuickBarComponent::MakeSlotData(int32 SlotIndex) const
{
	FQuickBarSlotData Data;
	Data.SlotIndex = SlotIndex;
	Data.Owner = GetOwner();

	if (!Slots.IsValidIndex(SlotIndex))
	{
		return Data;
	}

	if (UInventoryItemInstance* ItemInstance = Slots[SlotIndex])
	{
		Data.InventoryItemInstance = ItemInstance;
		Data.InventoryDef = ItemInstance->GetItemDef();

		if (const UInventoryFragment_EquippableItem* EquipFrag = ItemInstance->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			TSubclassOf<UEquipmentDefinition> EquipmentDefinition = EquipFrag->EquipmentDefinition;
			if (EquipmentDefinition != nullptr)
			{
				Data.EquipmentDef = EquipmentDefinition;
				if (UEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
				{
					Data.EquipmentInstance = EquipmentManager->GetEquipmentInstanceByDefinition(EquipmentDefinition);
				}
			}
		}
	}

	return Data;
}
