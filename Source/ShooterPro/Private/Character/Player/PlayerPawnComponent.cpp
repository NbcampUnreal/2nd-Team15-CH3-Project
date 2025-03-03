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


	//EquipmentManager = CreateDefaultSubobject<UEquipmentManagerComponent>(TEXT("EquipmentManager"));
}

void UPlayerPawnComponent::BeginPlay()
{
	Super::BeginPlay();

}