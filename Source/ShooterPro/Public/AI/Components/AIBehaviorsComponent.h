#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "AI/Utility/ShooterProUtility.h"
#include "Components/ActorComponent.h"
#include "AIBehaviorsComponent.generated.h"


UCLASS(ClassGroup=("AI"), meta=(BlueprintSpawnableComponent))
class SHOOTERPRO_API UAIBehaviorsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIBehaviorsComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



public:
	UFUNCTION(BlueprintPure, Category="AI Behavior")
	float GetAttackRadius() const { return AttackRadius; }

	UFUNCTION(BlueprintPure, Category="AI Behavior")
	float GetDefendRadius() const { return DefendRadius; }

private:
	UPROPERTY(EditAnywhere, Category="AI Behavior")
	float AttackRadius = 200.0f;

	UPROPERTY(EditAnywhere, Category="AI Behavior")
	float DefendRadius = 400.0f;
};
