// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "PlayerPawnComponent.generated.h"

class UGameplayAbility;
struct FInputActionValue;
class UCameraComponent;
class USpringArmComponent;
class UEquipmentManagerComponent;
/**
 * 
 */
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class SHOOTERPRO_API UPlayerPawnComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UPlayerPawnComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

private:
	
	
};
