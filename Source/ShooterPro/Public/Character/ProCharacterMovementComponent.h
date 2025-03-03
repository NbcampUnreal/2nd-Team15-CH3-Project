// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UProCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UProCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	float GetGroundDistance();

	float GroundTraceDistance = 100000.f;
};
