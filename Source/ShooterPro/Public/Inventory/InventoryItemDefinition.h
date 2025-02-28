// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryItemDefinition.generated.h"

class UInventoryItemInstance;

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class SHOOTERPRO_API UInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UInventoryItemInstance* Instance) const {}
};

UCLASS(Blueprintable, Const, Abstract)
class SHOOTERPRO_API UInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	UInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Equipment, Instanced)
	TArray<TObjectPtr<UInventoryItemFragment>> Fragments;

	const UInventoryItemFragment* FindFragmentByClass(TSubclassOf<UInventoryItemFragment> FragmentClass) const;
};
