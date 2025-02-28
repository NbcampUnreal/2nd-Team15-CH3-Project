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
	// �� AI�� ��ȯ�ϴ� ��ų ���� �� �Լ��� ȣ���Ͽ� ���� ����
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
	// ������ų ���� ��ġ�� ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionExtent;

	// The class of the Actor to spawn
	// ������ų ���� Ŭ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	TSubclassOf<AActor> SpawnClass;
	// Minimum number of Actors to spawn
	// ������ų ������ �ּҰ�
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 SpawnAmountMin;
	// Maximum number of Actors to spawn
	// ������ų ������ �ִ밪
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	int32 SpawnAmountMax;
	// Default spawn height(Z offset)
	// �⺻ ���� ����(���� ����)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	float SpawnHeight;
	// Whether infinite spawning is possible. If true, you can spawn multiple times, even if you've already spawned.
	// ���� ���� ���ɿ���. ���� ��� �̹� �����ߴ��� �ߺ� ������ �� �ֽ��ϴ�.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bInfinitySpawnMode;

protected:
	int32 AliveActors;
	int32 SpawnAmount;
};