// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EquipmentDefinition.h"
#include "EquipmentInstance.generated.h"

struct FEquipmentActorToSpawn;


UCLASS(BlueprintType, Blueprintable)
class SHOOTERPRO_API UEquipmentInstance : public UObject
{
	GENERATED_BODY()
public:
	UEquipmentInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	UEquipmentInstance(UEquipmentDefinition* EquipDef, const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintNativeEvent,Category="Equipment")
	void OnInstanceCreated();

	virtual UWorld* GetWorld() const override;

	UFUNCTION(BlueprintPure, Category = "Equipment")
	APawn* GetOwnerAsPawn() const;

	UFUNCTION(BlueprintPure, Category = "Equipment", meta=(DeterminesOutputType=Type))
	APawn* GetOwnerAsType(TSubclassOf<APawn> Type) const;

	template<typename T>
	T* GetOwnerAsTypeTemp() const;
	
	virtual void SpawnEquipmentActors(const TArray<FEquipmentActorToSpawn>& ActorsToSpawn);
	virtual void DestroyEquipmentActors();
	
	virtual void OnEquipped();
	virtual void OnUnequipped();

	UFUNCTION(BlueprintImplementableEvent,Category="Equipment")
	void K2_OnEquipped();
	
	UFUNCTION(BlueprintImplementableEvent,Category="Equipment")
	void K2_OnUnequipped();
	
	UFUNCTION(BlueprintPure, Category=Equipment)
	UObject* GetInstigator() const { return Instigator; }
	
	void SetInstigator(UObject* InInstigator) {Instigator = InInstigator;}
	
	void SetAnimMontage(UAnimMontage* Montage);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	AActor* FindSpawnedActorByClass(TSubclassOf<AActor> Class) const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UEquipmentDefinition* GetEquipmentDefinition() const {return EquipmentDefinition;}
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Animation)
	UAnimMontage* EquippedAnimMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Animation)
	UAnimMontage* UnequippedAnimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Animation)
	TSubclassOf<UAnimInstance> EquippedLayer;
	
	UPROPERTY(BlueprintReadOnly, Category=SpawnedActors)
	TArray<TObjectPtr<AActor>> SpawnedActors;

	UPROPERTY()
	UEquipmentDefinition* EquipmentDefinition;
	
	UPROPERTY()
	TObjectPtr<UObject> Instigator;
};

template <typename T>
T* UEquipmentInstance::GetOwnerAsTypeTemp() const
{
	return Cast<T>(GetOwnerAsPawn());
}
