#pragma once

#include "CoreMinimal.h"

#include "AI/EnemyAITypes.h"

#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnemyAIBluePrintFunctionLibrary.generated.h"

class AEnemyAIBase;
/**
 * 
 */
UCLASS()
class SHOOTERPRO_API UEnemyAIBluePrintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	/************************************************************************/
	/*							BLACKBOARD KEYS                             */
	/************************************************************************/
protected:
	static const FName BBKeyName_PreviousState;
	static const FName BBKeyName_CurrentState;
	static const FName BBKeyName_AttackRadius;
	static const FName BBKeyName_DefendRadius;
	static const FName BBKeyName_PointOfInterest;
	static const FName BBKeyName_AttackTarget;
	static const FName BBKeyName_StartLocation;
	static const FName BBKeyName_MaxRangeRadius;
	static const FName BBKeyName_RandomInt;
	static const FName BBKeyName_HasActivableAbility;

public:
	/** Returns blackboard key name */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_HasActivableAbility() { return BBKeyName_HasActivableAbility; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_PreviousState() { return BBKeyName_PreviousState; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_CurrentState() { return BBKeyName_CurrentState; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_StartLocation() { return BBKeyName_StartLocation; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_MaxRangeRadius() { return BBKeyName_MaxRangeRadius; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_RandomInt() { return BBKeyName_RandomInt; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_AttackRadius() { return BBKeyName_AttackRadius; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_DefendRadius() { return BBKeyName_DefendRadius; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_PointOfInterest() { return BBKeyName_PointOfInterest; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_AttackTarget() { return BBKeyName_AttackTarget; }
};
