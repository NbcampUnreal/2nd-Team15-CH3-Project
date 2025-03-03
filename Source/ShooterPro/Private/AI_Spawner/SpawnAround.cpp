// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Spawner/SpawnAround.h"
#include "Components/BoxComponent.h"

// Sets default values for this component's properties
USpawnAround::USpawnAround()
{
	PrimaryComponentTick.bCanEverTick = false;

	CollisionExtent = 100.f;

	SpawnClass = nullptr;
	SpawnAmountMin = 3;
	SpawnAmountMax = 5;
	SpawnHeight = 100.f;
	bInfinitySpawnMode = false;

	AliveActors = 0;
	SpawnAmount = 0;
}

void USpawnAround::SpawnLoop()
{
	SpawnAmount = FMath::RandRange(SpawnAmountMin, SpawnAmountMax);

	if(bInfinitySpawnMode)
	{
		for (int i = 0; i < SpawnAmount; i++)
		{
			StartSpawn();
		}
	}
	else
	{
		int32 SpawnToFill = SpawnAmount - AliveActors;

		for(int i = 0; i < SpawnToFill; i++)
		{
			StartSpawn();
		}
	}
}

void USpawnAround::StartSpawn()
{
	FTransform SpawnPoint = GetRandomSpawnPoint();

	SpawnActor(SpawnPoint);
}

FTransform USpawnAround::GetRandomSpawnPoint()
{
	FVector BoxExtent(CollisionExtent, CollisionExtent, 0.f);
	FVector BoxOrigin;
	if (AActor* Owner = GetOwner())
	{
		BoxOrigin = Owner->GetActorLocation();
	}

	FVector PointLocation = BoxOrigin + FVector(
		FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		SpawnHeight
	);

	FRotator PointLotation = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

	return FTransform(PointLotation, PointLocation, FVector(1.f, 1.f, 1.f));
}

void USpawnAround::SpawnActor(FTransform Trans)
{
	if (!GetWorld()) return;
	if (!SpawnClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawnClass, Trans, SpawnParams);

	if (SpawnedActor)
	{
		AliveActors++;
		SpawnedActor->OnDestroyed.AddDynamic(this, &USpawnAround::ActorWasKilled);
		return;
	}

	StartSpawn();
}

void USpawnAround::ActorWasKilled(AActor* DestroyedActor)
{
	AliveActors--;

	if (AliveActors <= 0)
	{
		AliveActors = 0;
	}
}

