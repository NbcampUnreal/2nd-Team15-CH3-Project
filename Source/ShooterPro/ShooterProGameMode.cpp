// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterProGameMode.h"
#include "ShooterProCharacter.h"
#include "UObject/ConstructorHelpers.h"

AShooterProGameMode::AShooterProGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
