// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ProCharacterBase.h"

#include "Components/GSCAbilityInputBindingComponent.h"
#include "Components/GSCCoreComponent.h"

// Sets default values
AProCharacterBase::AProCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CoreComponent = CreateDefaultSubobject<UGSCCoreComponent>(TEXT("Core"));
	InputBindingComponent = CreateDefaultSubobject<UGSCAbilityInputBindingComponent>(TEXT("InputBinding"));
}

// Called when the game starts or when spawned
void AProCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AProCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

