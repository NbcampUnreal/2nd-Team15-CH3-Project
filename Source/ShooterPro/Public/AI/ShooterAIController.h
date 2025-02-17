#pragma once

#include "CoreMinimal.h"
#include "ShooterAITypes.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "ShooterAIController.generated.h"

class UAIBehaviorsComponent;
class AShooterAIBase;

UCLASS()
class SHOOTERPRO_API AShooterAIController : public AAIController
{
	GENERATED_BODY()

public:
	AShooterAIController();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION()
	void UpdateTarget();

	UFUNCTION(BlueprintCallable, Category="ShooterAIController")
	void UpdateSenseToTarget();

	UFUNCTION(BlueprintCallable, Category="ShooterAIController")
	void UpdateSenseToTargetCompanion();

	UFUNCTION(BlueprintCallable, Category="ShooterAIController")
	void SetTarget(AActor* NewTarget, bool Companion = false);

protected:
	UFUNCTION()
	void OnCombatChanged(bool bNewIsCombat);

public:
	void BlackBoardUpdate_IsInCombat(bool bNewIsCombat);
	void BlackBoardUpdate_Behavior(EAIBehavior NewBehavior);
	void BlackBoardUpdate_Target(UObject* NewObject);

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="ShooterAIController")
	TObjectPtr<UAIPerceptionComponent> AIPerception;
	
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ShooterAIController")
	TObjectPtr<AActor> Target;	

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ShooterAIController|Reference")
	TObjectPtr<AShooterAIBase> AIPossessed;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ShooterAIController|Reference")
	TObjectPtr<UAIBehaviorsComponent> PossessedBehaviorsComp;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="ShooterAIController")
	bool bIsInCombat;

private:
	FTimerHandle UpdateTimerHandle;
};
