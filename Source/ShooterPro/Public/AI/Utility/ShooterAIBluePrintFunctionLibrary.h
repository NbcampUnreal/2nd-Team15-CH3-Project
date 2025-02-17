#pragma once

#include "CoreMinimal.h"

#include "AI/ShooterAITypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShooterAIBluePrintFunctionLibrary.generated.h"

class AShooterAIBase;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UShooterAIBluePrintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	/************************************************************************/
	/*							BLACKBOARD KEYS                             */
	/************************************************************************/
protected:
	static const FName BBKeyName_InCombat;
	static const FName BBKeyName_Behavior;
	static const FName BBKeyName_Target;

public:
	/** Returns blackboard key name */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_InCombat() { return BBKeyName_InCombat; }
	static const FName& GetBBKeyName_Behavior() { return BBKeyName_Behavior; }
	static const FName& GetBBKeyName_Target() { return BBKeyName_Target; }

	//ToDo: Implement this function
	static EHitDirection GetHitDirection(const FVector_NetQuantizeNormal& ShotDirection, AShooterAIBase* ShooterAIBase);
};
