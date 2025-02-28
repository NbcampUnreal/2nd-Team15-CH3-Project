// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterProPlayerController.generated.h"

class UInventoryItemInstance;
class UInputAction;
class UInputMappingContext;


UCLASS()
class SHOOTERPRO_API AShooterProPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AShooterProPlayerController();

	virtual void BeginPlay() override;

	void CreateWidgets();
	void ShowWidgets();
	void ClearAllWidgets();
	
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
	
	UFUNCTION(BlueprintCallable, Category=UI)
	void UpdateQuickBar();

	UFUNCTION(BlueprintCallable, Category=UI)
	void UpdateSlot(UUserWidget* Slot, const UInventoryItemInstance* Item);

	UFUNCTION(BlueprintCallable, Category=UI)
	void QuickBarSlotChanged(int32 NewSlot);

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HUD")
	TSubclassOf<UUserWidget> UserWidgetClass;

	UPROPERTY()
	UUserWidget* UserWidget;
};

