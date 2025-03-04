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

	/*UFUNCTION(BlueprintCallable, Category = "GameLoop")
	void SetSpawnPoint(int32 RespawnPointIndex);*/

	UFUNCTION(BlueprintPure, Category = "GameLoop")
	TMap<FGameplayTag, int32> GetKilledEnemyMap() const { return KilledEnemyMap; }

	/*UFUNCTION(BlueprintPure, Category = "GameLoop")
	TArray<FVector> GetRespawnPoints() const { return RespawnPoints; }*/

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	void ShowGameClearWidget();

	UFUNCTION(BlueprintImplementableEvent, Category = "Widget")
	void ShowGameOverWidget();

protected:
	TMap<FGameplayTag, int32> KilledEnemyMap;

	/*TArray<FVector> RespawnPoints;

	int32 CurrentRespawnPoint;*/

	TObjectPtr<AProPlayerCharacter> PlayerCharacter;
};
