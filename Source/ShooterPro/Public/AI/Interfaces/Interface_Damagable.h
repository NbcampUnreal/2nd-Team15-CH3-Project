// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interface_Damagable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInterface_Damagable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SHOOTERPRO_API IInterface_Damagable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	int32 GetTeamNumber();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	bool IsDead();
};
