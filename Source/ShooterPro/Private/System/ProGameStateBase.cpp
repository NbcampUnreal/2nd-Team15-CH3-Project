// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ProGameStateBase.h"
#include "Character/Player/ProPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void AProGameStateBase::BeginPlay()
{
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
	//PlayerController->SetPause(true);

	if (!IsDead)
	{
		//ShowGameClearWidget();
	}
	else
	{
		//ShowGameOverWidget();
	}
}
