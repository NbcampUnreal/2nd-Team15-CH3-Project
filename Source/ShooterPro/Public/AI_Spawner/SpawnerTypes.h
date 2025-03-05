// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "AIController.h"
#include "SpawnerTypes.generated.h"

// ----------------------------- ENUM -----------------------------
/**
 * Define the location of the Actor to spawn.
 * ������ ������ ��ġ�� �����մϴ�.
 */
UENUM(BlueprintType)
enum class EAISpawnPointType : uint8
{
	None									UMETA(ToolTip = "Don't spawn at all"),
	UseRandomPoints					UMETA(ToolTip = "Spawn at a random point")
};

/**
 * Define how Actors spawn.
 * ������ ���� ����� �����մϴ�.
 */
UENUM(BlueprintType)
enum class EAISpawnMethod: uint8
{
	None						UMETA(ToolTip = "Don't spawn at all"),
	SpawnOnGameStart	UMETA(ToolTip = "Spawn right after game starts ( On Begin Play )"),
	SpawnOnRadius		UMETA(ToolTip = "Spawn when Invoker(Player) enters specified SpawnRadius")
};

/**
 * Define how Actors respawn.
 * ������ ������ ����� �����մϴ�.
 */
UENUM(BlueprintType)
enum class EAIReSpawnMethod : uint8
{
	None							UMETA(ToolTip = "Don't respawn"),
	RespawnOnIndividual	UMETA(ToolTip = "Respawn whenever an Actor is destroyed"),
	RespawnOnAll				UMETA(ToolTip = "Respawn when all Actors are destroyed")
};

/**
 * The Actor's enabling items to optimize.
 * ����ȭ��ų ������ Ȱ��ȭ �׸��Դϴ�.
 */
UENUM(BlueprintType, meta = (Bitflags))
enum class EAIOptimizerFlags : uint8
{
	AIBrain						UMETA( ToolTip = "Behavior Tree / Logic" ),
	MovementComponent	UMETA( ToolTip = "Character Movement Component" ),
	Visibility						UMETA( ToolTip = "Visibility of the Actor ( Hide/Show In Game )" ),
	Collision						UMETA( ToolTip = "Collision enabled on the Actor" ),
	Animations					UMETA( ToolTip = "Pause/Stop playing animation blueprint" ),
	ActorTick						UMETA( ToolTip = "Enable/Disable Tick function running on Actor" ),
	Shadows						UMETA( ToolTip = "Show/Hide shadows of Actor" )
};

// ----------------------------- STRUCT -----------------------------

/**
 * The group of Actors to spawn in the spawner.
 * �����ʿ��� ������ų ������ �׷��Դϴ�.
 */
USTRUCT(BlueprintType)
struct FAIPendingSpawnGroup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawner)
	int32 SpawnedAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawner)
	int32 TotalAmountToSpawn = 0;
};

/**
 * The data table structure to receive the spawn list.
 * ���� ����� ���� ������ ���̺� ����ü�Դϴ�.
 */
USTRUCT(BlueprintType)
struct FAISpawnRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> SpawnClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnChance;
};