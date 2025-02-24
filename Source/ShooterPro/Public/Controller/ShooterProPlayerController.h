// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterProPlayerController.generated.h"

class UQuickBarComponent;
class UInventoryManagerComponent;
class UInputAction;
class UInputMappingContext;


UCLASS()
class SHOOTERPRO_API AShooterProPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterProPlayerController();

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> CrouchAction;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Input)
	TObjectPtr<UInputAction> JumpAction;
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Inventory)
	TObjectPtr<UInventoryManagerComponent> InventoryManagerComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Inventory)
	TObjectPtr<UQuickBarComponent> QuickBarComponent;
};
