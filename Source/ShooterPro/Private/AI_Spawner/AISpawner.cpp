// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Spawner/AISpawner.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
//#include "NavigationSystem.h"

AAISpawner::AAISpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// 컴포넌트 생성
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(SceneComponent);
	SpawnCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SpawnCollision->SetupAttachment(SceneComponent);
	DetectCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	DetectCollision->SetupAttachment(SceneComponent);

	// 프로퍼티 초기화
	SpawnAmount = 10;
	SpawnHeight = 100.f;
	SpawnDelay = 0.f;
	SpawningInterval = 0.1f;
	ActorsSpawnedThisFrame = 0;
	SpawningCapacity = 1;

	SpawnPointType = EAISpawnPointType::UseRandomPoints;
	SpawnMethod = EAISpawnMethod::SpawnOnGameStart;
	RespawnMethod = EAIReSpawnMethod::None;

	DetectRadius = 500.f;
	SpawnRadius = 200.f;

	// 스폰 수에 대한 정보를 관리하는 구조체 초기화
	PendingSpawnGroup.SpawnedAmount = 0;
	PendingSpawnGroup.TotalAmountToSpawn = 0;

	// 상태 변수 초기화
	bIsSpawning = false;

	TotalSpawnedActors = 0;
	TotalAliveActors = 0;
}

void AAISpawner::BeginPlay()
{
	Super::BeginPlay();
	
		if (SpawnDelay == 0.f)
		{
			InitSpawner();
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(InitSpawnTimer, this, &AAISpawner::InitSpawner, SpawnDelay, false);
		}
}

void AAISpawner::OnConstruction(const FTransform& Transform)
{
	DetectCollision->SetSphereRadius(DetectRadius);
	SpawnCollision->SetBoxExtent(FVector(SpawnRadius, SpawnRadius, SpawnRadius));
}

bool AAISpawner::bIsPlayerInRadius()
{
	float ClosetPlayerDistanceSquared = FLT_MAX;

	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	float CurrentPlayerDistanceSquared = UKismetMathLibrary::Vector_DistanceSquared(GetActorLocation(), PlayerLocation);

	if (CurrentPlayerDistanceSquared < ClosetPlayerDistanceSquared)
	{
		ClosetPlayerDistanceSquared = CurrentPlayerDistanceSquared;
	}

	if (DetectRadius * DetectRadius >= ClosetPlayerDistanceSquared)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Player Detected!"));
		}

		return true;
	}

	return false;
}

bool AAISpawner::bCanSpawnActor()
{
	if (SpawnGroups.IsValidIndex(0))
	{
		return true;
	}
	
	return false;

	//switch (SpawnMethod)
	//{
	//case EAISpawnMethod::None:
	//	return true;
	//	break;

	//case EAISpawnMethod::SpawnOnGameStart:
	//	return true;
	//	break;

	//case EAISpawnMethod::SpawnOnRadius:
	//	return bIsPlayerInRadius();
	//	break;

	//default:
	//	return false;
	//	break;
	//}
}



// SpawnMethod에 따라 스폰 방식 선택
void AAISpawner::InitSpawner()
{
	GetWorld()->GetTimerManager().ClearTimer(InitSpawnTimer);

	switch (SpawnMethod)
	{
	case EAISpawnMethod::None:
		if (GEngine)
		{
			FString SwitchErrorMsg = FString::Printf(TEXT("The spawn method is none. Do not spawn."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, SwitchErrorMsg);
		}
		break;

	case EAISpawnMethod::SpawnOnGameStart:
		AddGroupToSpawn(SpawnAmount);
		break;

	case EAISpawnMethod::SpawnOnRadius:
		InitSpawnByRadius();
		break;

	default:
		if (GEngine)
		{
			FString SwitchErrorMsg = FString::Printf(TEXT("Unknown SpawnMethod"));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, SwitchErrorMsg);
		}
		break;
	}
}

void AAISpawner::AddGroupToSpawn(int32 Amount)
{
	if (SpawnAmount <= 0) return;

	PendingSpawnGroup.TotalAmountToSpawn = Amount;
	PendingSpawnGroup.SpawnedAmount = 0;
	SpawnGroups.Add(PendingSpawnGroup);

	TryToSpawnGroup();
}

void AAISpawner::InitSpawnByRadius()
{
	GetWorld()->GetTimerManager().SetTimer(CheckRadiusTimer, this, &AAISpawner::CheckRadius, 0.25f, true);
}


void AAISpawner::TryToSpawnGroup()
{
	if (bCanSpawnActor())
	{/*
		if (!bIsSpawning)
		{*/
			bIsSpawning = true;

			SpawningLoop();
		//}
	}
}

void AAISpawner::SpawningLoop()
{
	SpawnAICharacter();

	TotalSpawnedActors++;
	TotalAliveActors++;
	SpawnGroups[0].SpawnedAmount++;
	
	if (SpawnGroups[0].SpawnedAmount == SpawnGroups[0].TotalAmountToSpawn)
	{
		FinishSpawningGroup();
	}
	else
	{
		ActorsSpawnedThisFrame++;

		if (ActorsSpawnedThisFrame >= SpawningCapacity)
		{
			ActorsSpawnedThisFrame = 0;

			if (SpawningInterval == 0.f)
			{
				SpawningLoop();
			}
			else
			{
				GetWorld()->GetTimerManager().SetTimer(SpawnLoopTimer, this, &AAISpawner::IntervalPart, SpawningInterval, false);
			}
		}
		else
		{
			IntervalPart();
		}
	}
}

// 
void AAISpawner::SpawningActor(FTransform Trans)
{
	if (!GetWorld()) return;

	/*UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSystem)
	{
		ANavigationData* NavData = NavSystem->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::DontCreate);
		if (NavData)
		{
			FNavLocation NavLocation;
			if (NavSystem->ProjectPointToNavigation(Trans.GetLocation(), NavLocation, FVector::ZeroVector, &NavData->GetConfig()))
			{*/
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
				AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawnClass, Trans, SpawnParams);

				if (SpawnedActor)
				{
					SpawnedActors.Add(SpawnedActor);
					SpawnedActor->OnDestroyed.AddDynamic(this, &AAISpawner::ActorWasKilled);
					return;
				}
			/*}
		}
	}*/

	SpawnAICharacter();
}

void AAISpawner::CheckRadius()
{
	if (bIsPlayerInRadius())
	{
		GetWorld()->GetTimerManager().ClearTimer(CheckRadiusTimer);
		AddGroupToSpawn(SpawnAmount);
	}
}

void AAISpawner::SpawnAICharacter()
{
	FTransform SpawnPoint;

	switch (SpawnPointType)
	{
	case EAISpawnPointType::None:
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, TEXT("Spawning stop. Not valid spawn point."));
		}
		break;

	case EAISpawnPointType::UseRandomPoints:
		SpawnPoint = GetRandomSpawnPoint();
		break;

	default:
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Error : Not Defined Spawn Point."));
		}
		break;
	}

	SpawningActor(SpawnPoint);
}

void AAISpawner::IntervalPart()
{/*
	if (bCanSpawnActor())
	{*/
		SpawningLoop();
	//}
	//else
	//{
	//	bIsSpawning = false;
	//}
}

void AAISpawner::FinishSpawningGroup()
{
	bIsSpawning = false;
	SpawnGroups.RemoveAt(0);
}

void AAISpawner::ActorWasKilled(AActor* DestroyedActor)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("ByeBye~"));
	}

	RespawnLoop();
}

void AAISpawner::RespawnLoop()
{
	switch (RespawnMethod)
	{
	case EAIReSpawnMethod::None:
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Do not respawn."));
		}
		break;

	case EAIReSpawnMethod::RespawnOnIndividual:
		TotalAliveActors--;
		IndividualRespawn();
		break;

	case EAIReSpawnMethod::RespawnOnAll:
		TotalAliveActors--;
		AllRespawn();
		break;

	default:
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Not valid respawn method!"));
		}
		break;
	}
}

void AAISpawner::IndividualRespawn()
{
	RespawnActors.Add(1);

	if (SpawnDelay == 0.f)
	{
		SetRespawn();
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(InitSpawnTimer, this, &AAISpawner::SetRespawn, SpawnDelay, false);
	}
}

void AAISpawner::AllRespawn()
{
	if(TotalAliveActors <= 0)
	{
		TotalAliveActors = 0;
		RespawnActors.Add(SpawnAmount);

		if (SpawnDelay == 0.f)
		{
			SetRespawn();
		}
		else
		{
			GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AAISpawner::SetRespawn, SpawnDelay, false);
		}
	}
}

void AAISpawner::SetRespawn()
{
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AAISpawner::Respawn, 1.f, true);
}

void AAISpawner::Respawn()
{
	if (RespawnActors.IsValidIndex(0))
	{
		AddGroupToSpawn(RespawnActors[0]);
		RespawnActors.RemoveAt(0);

		if (!RespawnActors.IsValidIndex(0))
		{
			GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
		}
	}
}

FTransform AAISpawner::GetRandomSpawnPoint()
{
	FVector BoxExtent = SpawnCollision->GetScaledBoxExtent();
	FVector BoxOrigin = SpawnCollision->GetComponentLocation();

	FVector PointLocation = BoxOrigin + FVector(
		FMath::FRandRange(-BoxExtent.X, BoxExtent.X),
		FMath::FRandRange(-BoxExtent.Y, BoxExtent.Y),
		SpawnHeight
	);

	FRotator PointLotation = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

	return FTransform(PointLotation, PointLocation, FVector(1.f, 1.f, 1.f));
}