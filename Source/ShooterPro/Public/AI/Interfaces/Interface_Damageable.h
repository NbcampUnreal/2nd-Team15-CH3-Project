// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "UObject/Interface.h"
#include "Interface_Damageable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UInterface_Damageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SHOOTERPRO_API IInterface_Damageable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	int32 GetTeamNumber();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	bool IsDead();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	bool TakeDamage(FDamageInfo DamageInfo, AActor* DamageCauser);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	float GetCurrentHealth();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	float GetMaxHealth();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	float Heal(float Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	bool IsAttacking();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	bool ReserveAttackToken(int Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	void ReturnAttackToken(int Amount);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	void SetIsInterruptible(bool bNewIsInterruptible);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Shooter Interface|Damagable")
	void SetIsInvincible(bool bNewIsInvincible);
};
