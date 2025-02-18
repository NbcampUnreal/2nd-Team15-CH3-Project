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
	static const FName BBKeyName_State;
	static const FName BBKeyName_AttackRadius;
	static const FName BBKeyName_DefendRadius;
	static const FName BBKeyName_PointOfInterest;
	static const FName BBKeyName_AttackTarget;

public:
	/** Returns blackboard key name */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_State() { return BBKeyName_State; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_AttackRadius() { return BBKeyName_AttackRadius; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_DefendRadius() { return BBKeyName_DefendRadius; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_PointOfInterest() { return BBKeyName_PointOfInterest; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ShooterAILibrary|AI")
	static const FName& GetBBKeyName_AttackTarget() { return BBKeyName_AttackTarget; }
};
