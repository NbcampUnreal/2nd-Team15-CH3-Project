// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ProHUD.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERPRO_API AProHUD : public AHUD
{
	GENERATED_BODY()
public:
	AProHUD();

	virtual void BeginPlay() override;

	// DECLARE_DYNAMIC_DELEGATE_OneParam(FQuickSlotChangedDelegate, int, NewSlot);
	
	// void ClearAllWidgets();
	//
	// void CreateWidgets();
	//
	// void ShowWidgets();
	//
	// void UpdateQuickBar();
protected:
	
	// UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HUD")
	// TSubclassOf<UUserWidget> UserWidgetClass;
	//
	// UPROPERTY()
	// UUserWidget* UserWidget;
};
