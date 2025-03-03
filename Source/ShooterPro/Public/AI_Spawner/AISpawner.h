// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI_Spawner/SpawnerTypes.h"
#include "AISpawner.generated.h"

class UBoxComponent;
class USphereComponent;
class AEnemyAIBase;

UCLASS()
class SHOOTERPRO_API AAISpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AAISpawner();

protected:
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;
	
	bool bIsPlayerInRadius();
	bool bCanSpawnActor();

	FAISpawnRow* GetRandomRow();

	// Spawn Logic
	// 스폰 로직
protected:
	void InitSpawner();

	void AddGroupToSpawn(int32 Amount);
	void InitSpawnByRadius();

	void TryToSpawnGroup();
	void SpawningLoop();
	void SpawningActor(FTransform Trans);

	void CheckRadius();

	void SpawnAICharacter();
	void IntervalPart();

	void FinishSpawningGroup();

	// Respawn Logic
	// 리스폰 로직
protected:
	UFUNCTION()
	void ActorWasKilled(AEnemyAIBase* DestroyedActor);

	void RespawnLoop();
	void IndividualRespawn();
	void AllRespawn();
	void SetRespawn();
	void Respawn();

	FTransform GetRandomSpawnPoint();

public:
	// The class of the Actor to spawn
	// 스폰시킬 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	UDataTable* SpawnDataTable;
	// Number of Actors to spawn
	// 스폰시킬 액터의 갯수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	int32 SpawnAmount;
	// Default spawn height(Z offset)
	// 기본 스폰 높이(높이 공간)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	float SpawnHeight;

	// Delay when executing new spawn commands
	// 새로운 스폰 명령 실행시 딜레이
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	float SpawnDelay;
	// Spawn interval between each Actor in a single command
	// 한 명령에서 각 액터간 스폰 간격
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	float SpawningInterval;
	// Number of spawns to run simultaneously
	// 동시에 실행시킬 스폰 수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	int32 SpawningCapacity;

	// Spawn at a random point
	// 랜덤 위치에 스폰
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Type")
	EAISpawnPointType SpawnPointType;
	// Spawn at when BeginPlay or Overlap Event
	// 게임 시작시 스폰하거나 오버랩시 스폰
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Type")
	EAISpawnMethod SpawnMethod;

	// Setting whether Actors respawn individually or all Actors respawn at once
	// 액터가 개별로 리스폰할지 모든 액터가 한번에 리스폰할지 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	EAIReSpawnMethod RespawnMethod;

	// Range to detect player character
	// 플레이어를 감지하는 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn On Radius")
	float DetectRadius;
	// Range to spawn AI character
	// AI 캐릭터가 랜덤하게 스폰되는 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn On Radius")
	float SpawnRadius;

protected:
	USceneComponent* SceneComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* SpawnCollision;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USphereComponent* DetectCollision;

	FAIPendingSpawnGroup PendingSpawnGroup;
	TArray<FAIPendingSpawnGroup> SpawnGroups;

	int32 ActorsSpawnedThisFrame;

	FTimerHandle InitSpawnTimer;
	FTimerHandle SpawnLoopTimer;
	FTimerHandle CheckRadiusTimer;
	FTimerHandle RespawnTimer;

	bool bIsSpawning;

	TArray<AActor*> SpawnedActors;
	TArray<AActor*> RespawnedActors;

	int32 TotalSpawnedActors;
	int32 TotalAliveActors;
};
