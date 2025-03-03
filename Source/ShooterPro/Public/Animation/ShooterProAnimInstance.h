// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Animation/AnimInstance.h"
#include "ShooterProAnimInstance.generated.h"
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UShooterProAnimInstance : public UAnimInstance
{
	GENERATED_BODY()


public:
	UShooterProAnimInstance(const FObjectInitializer& ObjectInitializer);

	virtual void InitializeWithAbilitySystem(UAbilitySystemComponent* ASC);

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:

	UPROPERTY(BlueprintReadOnly)
	float GroundDistance = -1.0f;

	UPROPERTY(EditDefaultsOnly, Category="GamplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
};