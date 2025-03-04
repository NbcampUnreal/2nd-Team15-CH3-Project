// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ProGameStateBase.h"
#include "Character/Player/ProPlayerCharacter.h"
#include "Controller/ShooterProPlayerController.h"
#include "Kismet/GameplayStatics.h"

void AProGameStateBase::BeginPlay()
{
	PlayerCharacter = nullptr;
	//CurrentRespawnPoint = 0;

	if (GetWorld())
	{
		PlayerCharacter = Cast<AProPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}

	StartLevel();
}

void AProGameStateBase::AddKillScore(FGameplayTag EnemyTag, int32 Amount)
{
	if (KilledEnemyMap.Contains(EnemyTag))
	{
		KilledEnemyMap[EnemyTag] += Amount;
	}
	else
	{
		KilledEnemyMap.Add(EnemyTag, Amount);
	}
}

void AProGameStateBase::StartLevel()
{
	KilledEnemyMap.Empty();
}

void AProGameStateBase::EndLevel(bool IsDead)
{
	ensure(PlayerCharacter);

	if (AShooterProPlayerController* PlayerController = Cast<AShooterProPlayerController>(PlayerCharacter->GetController()))
	{
		PlayerController->SetPause(true);
	}

	if (!IsDead)
	{
		ShowGameClearWidget();
	}
	else
	{
		ShowGameOverWidget();
	}
}

//void AProGameStateBase::SetSpawnPoint(int32 RespawnPointIndex)
//{
//	UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass());
//}
