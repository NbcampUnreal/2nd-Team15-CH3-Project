// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Spawner/AISpawner.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "AI_Spawner/AIOptimizerComponent.h"
#include "ShooterPro/Public/AI/EnemyAIBase.h"
//#include "NavigationSystem.h"

AAISpawner::AAISpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// ������Ʈ ����
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(SceneComponent);
	SpawnCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SpawnCollision->SetupAttachment(SceneComponent);
	DetectCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	DetectCollision->SetupAttachment(SceneComponent);

	// ������Ƽ �ʱ�ȭ
	SpawnDataTable = nullptr;
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

	// ���� ���� ���� ������ �����ϴ� ����ü �ʱ�ȭ
	PendingSpawnGroup.SpawnedAmount = 0;
	PendingSpawnGroup.TotalAmountToSpawn = 0;

	// ���� ���� �ʱ�ȭ
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

	//ToDo::임시땜빵코드
	if (!IsValid(PlayerCharacter))
		return false;

	FVector PlayerLocation = PlayerCharacter->GetActorLocation();

	float CurrentPlayerDistanceSquared = UKismetMathLibrary::Vector_DistanceSquared(DetectCollision->GetComponentLocation(), PlayerLocation);

	if (CurrentPlayerDistanceSquared < ClosetPlayerDistanceSquared)
	{
		ClosetPlayerDistanceSquared = CurrentPlayerDistanceSquared;
	}

	if (DetectRadius * DetectRadius >= ClosetPlayerDistanceSquared)
	{
		if (GEngine)
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

FAISpawnRow* AAISpawner::GetRandomRow()
{
	if (!SpawnDataTable) return nullptr;

	TArray<FAISpawnRow*> Rows;
	const FString ContextString(TEXT("AISpawnerContext"));
	SpawnDataTable->GetAllRows(ContextString, Rows);

	float TotalChance = 0.0f;
	for (FAISpawnRow* temp : Rows)
	{
		TotalChance += temp->SpawnChance;
	}

	float RandomChance = FMath::FRandRange(0.0f, TotalChance);
	float ChanceTemp = 0.0f;
	for (FAISpawnRow* temp : Rows)
	{
		ChanceTemp += temp->SpawnChance;

		if (ChanceTemp >= RandomChance)
		{
			return temp;
		}
	}

	return nullptr;
}


// SpawnMethod�� ���� ���� ��� ����
void AAISpawner::InitSpawner()
{
	GetWorld()->GetTimerManager().ClearTimer(InitSpawnTimer);

	if (SpawnDataTable == nullptr) return;

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
	{
		/*
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

	if (FAISpawnRow* Row = GetRandomRow())
	{
		if (UClass* SpawningClass = Row->SpawnClass.Get())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(SpawningClass, Trans, SpawnParams);

			if (SpawnedActor)
			{
				SpawnedActors.Add(SpawnedActor);
				AEnemyAIBase* Enemy = Cast<AEnemyAIBase>(SpawnedActor);
				if (Enemy)
				{
					Enemy->OnDestroyed.AddDynamic(this, &AAISpawner::ActorWasKilled);
				}
				return;
			}
		}
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
{
	/*
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
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("ByeBye~"));
	}*/

	RespawnedActors.Add(DestroyedActor);

	/*UAIOptimizerComponent* OptimizerComp = DestroyedActor->FindComponentByClass<UAIOptimizerComponent>();
	if (OptimizerComp)
	{
		OptimizerComp->OptimizerCheckerStop();
	}*/

	RespawnLoop();
}

void AAISpawner::RespawnLoop()
{
	switch (RespawnMethod)
	{
	case EAIReSpawnMethod::None:
		/*if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Do not respawn."));
		}*/
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
	if (TotalAliveActors <= 0)
	{
		TotalAliveActors = 0;

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
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AAISpawner::Respawn, SpawningInterval, true);
}

void AAISpawner::Respawn()
{
	if (RespawnedActors.IsValidIndex(0))
	{
		/*RespawnedActors[0]->SetActorTransform(GetRandomSpawnPoint());
		RespawnedActors[0]->OnPooledRespawn();

		UAIOptimizerComponent* OptimizerComp = RespawnedActors[0]->FindComponentByClass<UAIOptimizerComponent>();
		if (OptimizerComp)
		{
			OptimizerComp->OptimizerChecker();
		}*/

			AddGroupToSpawn(1);

		RespawnedActors.RemoveAt(0);

		if (!RespawnedActors.IsValidIndex(0))
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
