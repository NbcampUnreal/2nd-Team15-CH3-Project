#pragma once

#include "CoreMinimal.h"
#include "EquipmentDefinition.h"
#include "EquipmentInstance.h"
#include "Abilities/GSCAbilitySet.h"
#include "Components/PawnComponent.h"
#include "EquipmentManagerComponent.generated.h"


class UGSCAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FEquipmentItem
{
	GENERATED_BODY()

public:
	FEquipmentItem()
	{
	}

	// The equipment class that got equipped
	UPROPERTY()
	TSubclassOf<UEquipmentDefinition> EquipmentDefinition;

	UPROPERTY()
	TObjectPtr<UEquipmentInstance> Instance = nullptr;

	
		
	UPROPERTY()
	FGSCAbilitySetHandle GrantedHandles;
};


USTRUCT(BlueprintType)
struct FEquipmentList
{
	GENERATED_BODY()

	FEquipmentList() : OwnerComponent(nullptr)
	{
	}

	FEquipmentList(UActorComponent* OwnerComponent) : OwnerComponent(OwnerComponent)
	{
	}

	UGSCAbilitySystemComponent* GetGSCAbilitySystemComponent() const;

	UEquipmentInstance* AddItem(TSubclassOf<UEquipmentDefinition> EquipmentDefinition);

	void RemoveItem(UEquipmentInstance* Instance);

private:
	friend class UEquipmentManagerComponent;

	UPROPERTY()
	TArray<FEquipmentItem> Items;
	
	UPROPERTY()
	TObjectPtr<UActorComponent> OwnerComponent;
};

UCLASS(BlueprintType, Const, meta = (BlueprintSpawnableComponent))
class SHOOTERPRO_API UEquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UEquipmentManagerComponent(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeComponent() override;
	
	virtual void UninitializeComponent() override;

	UFUNCTION(BlueprintCallable, Category="Equipment")
	UEquipmentInstance* EquipItem(TSubclassOf<UEquipmentDefinition> EquipmentDefinition);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void UnequipItem(UEquipmentInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UEquipmentInstance* GetFirstInstanceOfType(TSubclassOf<UEquipmentInstance> InstanceType);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<UEquipmentInstance*> GetEquipmentInstancesOfType(TSubclassOf<UEquipmentInstance> InstanceType) const;
	
	UFUNCTION(BlueprintCallable, Category="Equipment")
	UEquipmentInstance* GetEquipmentInstanceByDefinition(TSubclassOf<UEquipmentDefinition> InDefinition) const;


private:
	UPROPERTY()
	FEquipmentList EquipmentList;
};
