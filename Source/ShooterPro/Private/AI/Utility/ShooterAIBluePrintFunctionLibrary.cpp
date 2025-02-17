

#include "AI/Utility/ShooterAIBluePrintFunctionLibrary.h"



const FName UShooterAIBluePrintFunctionLibrary::BBKeyName_InCombat = FName( "InCombat" );
const FName UShooterAIBluePrintFunctionLibrary::BBKeyName_Behavior = FName( "Behavior" );
const FName UShooterAIBluePrintFunctionLibrary::BBKeyName_Target = FName( "Target" );


EHitDirection UShooterAIBluePrintFunctionLibrary::GetHitDirection(const FVector_NetQuantizeNormal& ShotDirection, AShooterAIBase* ShooterAIBase)
{
	return EHitDirection::Front;
}
