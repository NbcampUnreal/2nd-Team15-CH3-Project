// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interface_CharacterStatus.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInterface_CharacterStatus : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SHOOTERPRO_API IInterface_CharacterStatus
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Character Status")
	bool IsAlive();
};
