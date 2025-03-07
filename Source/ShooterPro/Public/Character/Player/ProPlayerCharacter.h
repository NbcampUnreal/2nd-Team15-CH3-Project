// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ProCharacterBase.h"
#include "ProPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UQuickBarComponent;
class UInventoryManagerComponent;
class UEquipmentManagerComponent;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API AProPlayerCharacter : public AProCharacterBase
{
	GENERATED_BODY()

public:
	AProPlayerCharacter();

	virtual void BeginPlay() override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void ToggleCrouch(const FInputActionValue& Value);

	void StartJump(const FInputActionValue& Value);

	UFUNCTION(BlueprintImplementableEvent)
	void ADS();

	UFUNCTION(BlueprintImplementableEvent)
	void Zoom();

	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<UCameraComponent> CameraComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	TObjectPtr<UEquipmentManagerComponent> EquipmentManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	TObjectPtr<UInventoryManagerComponent> InventoryManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	TObjectPtr<UQuickBarComponent> QuickBarComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	bool IsAiming;
};
