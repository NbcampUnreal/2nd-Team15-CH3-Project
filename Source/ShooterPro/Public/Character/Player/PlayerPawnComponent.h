// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "PlayerPawnComponent.generated.h"

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

	void Initialize();
private:
	void InitializeInput(UInputComponent* InputComponent);
	
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Crouch(const FInputActionValue& Value);

	void Jump(const FInputActionValue& Value);
	
	UPROPERTY()
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	//UPROPERTY()
	//TObjectPtr<UEquipmentManagerComponent> EquipmentManager;
};
