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
	// ���� ����
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
	// ������ ����
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
	// ������ų ���� Ŭ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	UDataTable* SpawnDataTable;
	// Number of Actors to spawn
	// ������ų ������ ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	int32 SpawnAmount;
	// Default spawn height(Z offset)
	// �⺻ ���� ����(���� ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	float SpawnHeight;

	// Delay when executing new spawn commands
	// ���ο� ���� ��� ����� ������
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	float SpawnDelay;
	// Spawn interval between each Actor in a single command
	// �� ��ɿ��� �� ���Ͱ� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	float SpawningInterval;
	// Number of spawns to run simultaneously
	// ���ÿ� �����ų ���� ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Property")
	int32 SpawningCapacity;

	// Spawn at a random point
	// ���� ��ġ�� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Type")
	EAISpawnPointType SpawnPointType;
	// Spawn at when BeginPlay or Overlap Event
	// ���� ���۽� �����ϰų� �������� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Type")
	EAISpawnMethod SpawnMethod;

	// Setting whether Actors respawn individually or all Actors respawn at once
	// ���Ͱ� ������ ���������� ��� ���Ͱ� �ѹ��� ���������� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn")
	EAIReSpawnMethod RespawnMethod;

	// Range to detect player character
	// �÷��̾ �����ϴ� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn On Radius")
	float DetectRadius;
	// Range to spawn AI character
	// AI ĳ���Ͱ� �����ϰ� �����Ǵ� ����
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
