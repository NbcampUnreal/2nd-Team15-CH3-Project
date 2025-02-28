// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/ProPlayerCharacter.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Controller/ShooterProPlayerController.h"
#include "Equipment/EquipmentManagerComponent.h"
#include "Equipment/QuickBarComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Inventory/InventoryManagerComponent.h"

AProPlayerCharacter::AProPlayerCharacter()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->TargetArmLength = 300;
	SpringArmComponent->bUsePawnControlRotation = true;
	SpringArmComponent->SetupAttachment(GetRootComponent());
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	EquipmentManager = CreateDefaultSubobject<UEquipmentManagerComponent>(TEXT("EquipmentManager"));
	InventoryManager = CreateDefaultSubobject<UInventoryManagerComponent>(TEXT("InventoryManager"));
	QuickBarComponent = CreateDefaultSubobject<UQuickBarComponent>(TEXT("QuickBar"));
}

void AProPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AProPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(InputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (AShooterProPlayerController* PlayerController = GetController<AShooterProPlayerController>())
		{
			EnhancedInputComponent->BindAction(
			PlayerController->MoveAction,
			ETriggerEvent::Triggered,
			this,
			&AProPlayerCharacter::Move
			);
			
			EnhancedInputComponent->BindAction(
			PlayerController->LookAction,
			ETriggerEvent::Triggered,
			this,
			&AProPlayerCharacter::Look
			);
			
			EnhancedInputComponent->BindAction(
			PlayerController->CrouchAction,
			ETriggerEvent::Triggered,
			this,
			&AProPlayerCharacter::ToggleCrouch
			);
			
			EnhancedInputComponent->BindAction(
			PlayerController->JumpAction,
			ETriggerEvent::Triggered,
			this,
			&AProPlayerCharacter::StartJump
			);
		}
	}
}

void AProPlayerCharacter::Move(const FInputActionValue& Value)
{
	AController* PlayerController = GetController();

	if (!ensure(PlayerController)) return;

	const FVector2D MoveValue = Value.Get<FVector2D>();
	const FRotator MovementRotation(0.f, PlayerController->GetControlRotation().Yaw, 0.0f);

	if (MoveValue.X != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		AddMovementInput(MovementDirection, MoveValue.X);
	}
	
	if (MoveValue.Y != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(MovementDirection, MoveValue.Y);
	}
}

void AProPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	if (LookValue.X != 0.0f)
	{
		AddControllerYawInput(LookValue.X);
	}

	if (LookValue.Y != 0.0f)
	{
		AddControllerPitchInput(LookValue.Y);
	}
}

void AProPlayerCharacter::ToggleCrouch(const FInputActionValue& Value)
{
	if (bIsCrouched) UnCrouch();
	else Crouch();
}

void AProPlayerCharacter::StartJump(const FInputActionValue& Value)
{
	Jump();
}
