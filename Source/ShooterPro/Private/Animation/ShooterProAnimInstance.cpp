// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/ShooterProAnimInstance.h"

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

	//const APlayerCharacter* Character = Cast<APlayerCharacter>(GetOwningActor());

	//if (!ensure(Character)) return;
	
	//UCharacterMovementComponent* MoveComp = CastChecked<UCharacterMovementComponent>(Character->GetCharacterMovement());
}
