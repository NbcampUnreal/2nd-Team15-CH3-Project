// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "System/GameplayTagStackContainer.h"
#include "Inventory/InventoryItemDefinition.h"
#include "InventoryItemInstance.generated.h"

class UInventoryItemDefinition;
struct FGamplayTag;


UCLASS(BlueprintType, Blueprintable)
class SHOOTERPRO_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TSubclassOf<UInventoryItemDefinition> GetItemDef() const { return ItemDef; }

	void SetItemDef(TSubclassOf<UInventoryItemDefinition> InDef) { ItemDef = InDef; }

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount) { StatTags.AddStack(Tag, StackCount); }
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)	{ StatTags.RemoveStack(Tag, StackCount); }
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetStatTagStackCount(FGameplayTag Tag) const	{ return StatTags.GetStackCount(Tag); }

	bool HasStatTag(FGameplayTag Tag) const	{ return StatTags.ContainsTag(Tag);	}
	
	friend struct FInventoryList;

	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UInventoryItemFragment* FindFragmentByClass(TSubclassOf<UInventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

private:
	UPROPERTY()
	FGameplayTagStackContainer StatTags;

	TSubclassOf<UInventoryItemDefinition> ItemDef;
};

inline const UInventoryItemFragment* UInventoryItemInstance::FindFragmentByClass(
	TSubclassOf<UInventoryItemFragment> FragmentClass) const
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}