// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/ShooterProPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Equipment/QuickBarComponent.h"
#include "Inventory/InventoryManagerComponent.h"

AShooterProPlayerController::AShooterProPlayerController()
{
	InventoryManagerComponent = CreateDefaultSubobject<UInventoryManagerComponent>(TEXT("InventoryManager"));
	QuickBarComponent = CreateDefaultSubobject<UQuickBarComponent>(TEXT("QuickBar"));
}

void AShooterProPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(GetLocalPlayer());
	if (ensure(LocalPlayer))
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		
		if (ensure(Subsystem))
		{
			if (ensure(MappingContext)) Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}
