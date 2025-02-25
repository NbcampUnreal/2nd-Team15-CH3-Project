// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/PlayerPawnComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/ProCharacterBase.h"
#include "Controller/ShooterProPlayerController.h"
#include "Equipment/EquipmentManagerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

UPlayerPawnComponent::UPlayerPawnComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->TargetArmLength = 300;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);

	//EquipmentManager = CreateDefaultSubobject<UEquipmentManagerComponent>(TEXT("EquipmentManager"));
}

void UPlayerPawnComponent::BeginPlay()
{
	Super::BeginPlay();

	Initialize();
}

void UPlayerPawnComponent::Initialize()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());

	if (!ensure(Character)) return;
	
	SpringArmComponent->AttachToComponent(Character->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	InitializeInput(Character->InputComponent);

	GiveAbilitiesToOwner();
}

void UPlayerPawnComponent::GiveAbilitiesToOwner()
{
	AProCharacterBase* Owner = GetPawn<AProCharacterBase>();
	if (!ensure(Owner)) return;

	if (!ensure(Owner->AbilitySystemComponent)) return;

	if (!ensure(Owner->AbilitySystemComponent->AbilityActorInfo.IsValid())) return;
	
	for (TSubclassOf<UGameplayAbility> Ability : Abilities)
	{
		Owner->AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
	}
}

void UPlayerPawnComponent::InitializeInput(UInputComponent* InputComponent)
{
	check(InputComponent);
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (AShooterProPlayerController* PlayerController = GetController<AShooterProPlayerController>())
		{
			EnhancedInputComponent->BindAction(
			PlayerController->MoveAction,
			ETriggerEvent::Triggered,
			this,
			&UPlayerPawnComponent::Move
			);
			
			EnhancedInputComponent->BindAction(
			PlayerController->LookAction,
			ETriggerEvent::Triggered,
			this,
			&UPlayerPawnComponent::Look
			);
			
			EnhancedInputComponent->BindAction(
			PlayerController->CrouchAction,
			ETriggerEvent::Triggered,
			this,
			&UPlayerPawnComponent::Crouch
			);
			
			EnhancedInputComponent->BindAction(
			PlayerController->JumpAction,
			ETriggerEvent::Triggered,
			this,
			&UPlayerPawnComponent::Jump
			);
		}
	}
	
}

void UPlayerPawnComponent::Move(const FInputActionValue& Value)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!ensure(Pawn)) return;

	AController* Controller = Pawn->GetController();

	if (!ensure(Controller)) return;

	const FVector2D MoveValue = Value.Get<FVector2D>();
	const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.0f);

	if (MoveValue.X != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
		Pawn->AddMovementInput(MovementDirection, MoveValue.X);
	}
	
	if (MoveValue.Y != 0.0f)
	{
		const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
		Pawn->AddMovementInput(MovementDirection, MoveValue.Y);
	}
}

void UPlayerPawnComponent::Look(const FInputActionValue& Value)
{
	APawn* Pawn = GetPawn<APawn>();
	if (!ensure(Pawn)) return;

	const FVector2D LookValue = Value.Get<FVector2D>();

	if (LookValue.X != 0.0f)
	{
		Pawn->AddControllerYawInput(LookValue.X);
	}

	if (LookValue.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(LookValue.Y);
	}
}

void UPlayerPawnComponent::Crouch(const FInputActionValue& Value)
{
}

void UPlayerPawnComponent::Jump(const FInputActionValue& Value)
{
}
