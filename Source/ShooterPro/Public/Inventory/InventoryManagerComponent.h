
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagerComponent.generated.h"

class UInventoryManagerComponent;
class UInventoryItemDefinition;
class UInventoryItemInstance;

struct FInventoryList;

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	FInventoryItem()
	{
	}

private:
	friend FInventoryList;
	friend UInventoryManagerComponent;

	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	//UPROPERTY()
	//int32 MaxStackCount; ItemDef에서 값을 가져와야함.
	
	UPROPERTY()
	int32 LastObservedCount = INDEX_NONE;
};


USTRUCT(BlueprintType)
struct FInventoryList
{
	GENERATED_BODY()

	FInventoryList(): OwnerComponent(nullptr)
	{
	}

	FInventoryList(UActorComponent* InOwnerComponent): OwnerComponent(InOwnerComponent)
	{
	}
	
	UInventoryItemInstance* AddItem(const TSubclassOf<UInventoryItemDefinition>& ItemDef, int32 StackCount);
	bool RemoveItem(const TSubclassOf<UInventoryItemDefinition>& ItemDef, int32 StackCount);
	
	void EraseItem(UInventoryItemInstance* Instance);

	TArray<UInventoryItemInstance*> GetAllItems() const;

	FInventoryItem FindFirstItemWithDefinition(const TSubclassOf<UInventoryItemDefinition>& ItemDef) const;

private:
	UPROPERTY()
	TMap<TSubclassOf<UInventoryItemDefinition>, FInventoryItem> Items;
	
	
	UPROPERTY()
	TObjectPtr<UActorComponent> OwnerComponent;
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHOOTERPRO_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	UInventoryItemInstance* AddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StakcCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void EraseItemInstance(UInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category="Inventory")
	UInventoryItemInstance* FindFirstItemInstanceByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef) const;

	UFUNCTION(BlueprintCallable, Category="Inventory")
	FInventoryItem FindFirstInventoryItemByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef) const;
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	bool ConsumeItemsByDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 NumToConsume);
private:
	FInventoryList InventoryList;
};
