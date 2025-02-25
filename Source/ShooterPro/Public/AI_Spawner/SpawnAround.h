// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpawnAround.generated.h"

class UBoxComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SHOOTERPRO_API USpawnAround : public UActorComponent
{
	GENERATED_BODY()

public:
	USpawnAround();

	// Call this function to execute spawning when using a skill that summons enemy AI
	// 적 AI를 소환하는 스킬 사용시 이 함수를 호출하여 스폰 실행
	UFUNCTION(BlueprintCallable)
	void SpawnLoop();

protected:
	void StartSpawn();
	FTransform GetRandomSpawnPoint();
	void SpawnActor(FTransform Trans);

	UFUNCTION()
	void ActorWasKilled(AActor* DestroyedActor);

public:
	// Range of random locations to spawn
	// 스폰시킬 랜덤 위치의 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionExtent;

	// The class of the Actor to spawn
	// 스폰시킬 액터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AActor> SpawnClass;
	// Minimum number of Actors to spawn
	// 스폰시킬 액터의 최소값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 SpawnAmountMin;
	// Maximum number of Actors to spawn
	// 스폰시킬 액터의 최대값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 SpawnAmountMax;
	// Default spawn height(Z offset)
	// 기본 스폰 높이(높이 공간)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnHeight;
	// Whether infinite spawning is possible. If true, you can spawn multiple times, even if you've already spawned.
	// 무한 스폰 가능여부. 참일 경우 이미 스폰했더라도 중복 스폰할 수 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bInfinitySpawnMode;

protected:
	int32 AliveActors;
	int32 SpawnAmount;
};