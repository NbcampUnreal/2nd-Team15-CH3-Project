// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ShooterProAnimInstance.h"

#include "Character/ProCharacterMovementComponent.h"
#include "Character/Player/ProPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ShooterPro/ShooterProCharacter.h"

UShooterProAnimInstance::UShooterProAnimInstance(const FObjectInitializer& ObjectInitializer)
{
}

void UShooterProAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	//if (AActor* OwningActor = GetOwningActor())
	//{
	//}
	
}

void UShooterProAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	const AProPlayerCharacter* Character = Cast<AProPlayerCharacter>(GetOwningActor());

	if (!Character) return;
	
	UProCharacterMovementComponent* MoveComp = CastChecked<UProCharacterMovementComponent>(Character->GetCharacterMovement());

	if (!MoveComp) return;
	
	GroundDistance = MoveComp->GetGroundDistance();
}
