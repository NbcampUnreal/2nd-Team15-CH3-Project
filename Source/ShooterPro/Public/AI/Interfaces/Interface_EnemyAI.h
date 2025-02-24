// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "UObject/Interface.h"
#include "Interface_EnemyAI.generated.h"

class APatrolPath;
// This class does not need to be modified.
UINTERFACE()
class UInterface_EnemyAI : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SHOOTERPRO_API IInterface_EnemyAI
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|EnemyAI")
	int32 GetTeamNumber();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|EnemyAI")
	bool IsDead();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|EnemyAI")
	APatrolPath* GetPatrolPath();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|EnemyAI")
	float SetMoveSpeed(EAIMovementSpeed NewMovementSpeed);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|EnemyAI")
	void JumpToDestination(FVector NewDestination);
};
