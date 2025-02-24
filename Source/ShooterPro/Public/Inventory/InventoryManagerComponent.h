// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryManagerComponent.generated.h"

class UInventoryItemDefinition;
class UInventoryItemInstance;

USTRUCT(BlueprintType)
struct FInventoryItem
{
	GENERATED_BODY()

	FInventoryItem() {}

private:
	friend FInventoryList;
	friend UInventoryManagerComponent;
	
	UPROPERTY()
	TObjectPtr<UInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY()
	int32 LastObservedCount = INDEX_NONE;
};


USTRUCT(BlueprintType)
struct FInventoryList
{
	GENERATED_BODY()

	FInventoryList(): OwnerComponent(nullptr) {}

	FInventoryList(UActorComponent* InOwnerComponent): OwnerComponent(InOwnerComponent) {}


	UInventoryItemInstance* AddItem(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StackCount);
	void RemoveItem(UInventoryItemInstance* Instance);
	
	TArray<UInventoryItemInstance*> GetAllItems() const;
	
private:
	UPROPERTY()
	TArray<FInventoryItem> Items;
	
	TObjectPtr<UActorComponent> OwnerComponent;
};









UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTERPRO_API UInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


	UFUNCTION(BlueprintCallable, Category="Inventory")
	UInventoryItemInstance* AddItemDefinition(TSubclassOf<UInventoryItemDefinition> ItemDef, int32 StakcCount = 1);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveItemInstance(UInventoryItemInstance* ItemInstance);
	
private:
	FInventoryList InventoryList;
};



