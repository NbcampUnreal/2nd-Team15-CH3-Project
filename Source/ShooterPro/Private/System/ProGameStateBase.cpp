// Fill out your copyright notice in the Description page of Project Settings.


#include "System/ProGameStateBase.h"
#include "Character/Player/ProPlayerCharacter.h"
#include "Controller/ShooterProPlayerController.h"
#include "Kismet/GameplayStatics.h"

void AProGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	PlayerCharacter = nullptr;

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
	ensure(PlayerCharacter);
	if (AShooterProPlayerController* PlayerController = Cast<AShooterProPlayerController>(PlayerCharacter->GetController()))
	{
		PlayerController->SetPause(false);
		PlayerController->SetShowMouseCursor(false);
	}
}

void AProGameStateBase::EndLevel(bool bIsDead)
{
	ensure(PlayerCharacter);

	if (AShooterProPlayerController* PlayerController = Cast<AShooterProPlayerController>(PlayerCharacter->GetController()))
	{
		PlayerController->SetPause(true);
		PlayerController->SetShowMouseCursor(true);
	}

	if (!bIsDead)
	{
		ShowGameClearWidget();
	}
	else
	{
		ShowGameOverWidget();
	}
}

void AProGameStateBase::RestartLevel()
{
	KilledEnemyMap.Empty();
	StartLevel();
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("L_ScaleDown"));
}

int32 AProGameStateBase::GetKilledEnemyAmount(FGameplayTagContainer EnemyTags) const
{
	int32 TotalAmount = 0;

	if (EnemyTags.IsEmpty())
	{
		return 0;
	}

	for (FGameplayTag EnemyTag : EnemyTags)
	{
		if (KilledEnemyMap.Contains(EnemyTag))
		{
			TotalAmount += KilledEnemyMap[EnemyTag];
		}
	}

	return TotalAmount;
}
