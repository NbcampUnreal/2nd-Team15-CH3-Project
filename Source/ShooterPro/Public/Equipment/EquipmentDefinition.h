// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EquipmentDefinition.generated.h"

class UEquipmentInstance;

USTRUCT()
struct FEquipmentActorToSpawn
{
	GENERATED_BODY()

	FEquipmentActorToSpawn()
	{}

	UPROPERTY(EditAnywhere, Category=Equipment)
	TSubclassOf<AActor> ActorToSpawn;

	UPROPERTY(EditAnywhere, Category=Equipment)
	FName AttachSocket;	

	UPROPERTY(EditAnywhere, Category=Equipment)
	FTransform AttachTransform;
};


UCLASS(Blueprintable, Const, Abstract, BlueprintType)
class SHOOTERPRO_API UEquipmentDefinition : public UObject
{
	GENERATED_BODY()
public:
	UEquipmentDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, Category = Equipment)
	TSubclassOf<UEquipmentInstance> InstanceType;

	//UPROPERTY(EditDefaultsOnly, Category = Equipment)
	//TArray<TSubclassOf<const U>>

	UPROPERTY(EditDefaultsOnly, Category = Equipment)
	TArray<FEquipmentActorToSpawn> ActorsToSpawn;
};
