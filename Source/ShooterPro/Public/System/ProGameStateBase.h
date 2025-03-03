// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GameplayTags.h"
#include "ProGameStateBase.generated.h"

class AProPlayerCharacter;

UCLASS()
class SHOOTERPRO_API AProGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "GameLoop")
	void AddKillScore(FGameplayTag EnemyTag, int32 Amount);

	void StartLevel();

	UFUNCTION(BlueprintCallable, Category = "GameLoop")
	void EndLevel(bool IsDead);

public:
	TMap<FGameplayTag, int32> KilledEnemyMap;

protected:
	TObjectPtr<AProPlayerCharacter> PlayerCharacter;
};
