#pragma once

#include "CoreMinimal.h"
#include "EnemyAITypes.h"
#include "Runtime/AIModule/Classes/AIController.h"
#include "EnemyAIController.generated.h"

struct FAIStimulus;
// Forward declarations
class UAIBehaviorsComponent;
class AEnemyAIBase;


UCLASS()
class SHOOTERPRO_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	virtual void OnUnPossess() override;

public:
	virtual void Tick(float DeltaTime) override;

public:
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	void SetStateAsAttacking(AActor* NewAttackTarget, bool bUseLastKnownAttackTarget);
	
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	void SetStateAsInvestigating(const FVector& Location);
	
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	void SetStateAsDead();
	
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	void SetStateAsFrozen();

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	void SeekAttackTarget();

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	void CheckIfForgottenSeenActor();

public:
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	bool CanSenseActor(AActor* Actor, EAISense SenseType, FAIStimulus& OutAIStimulus);

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	bool OnSameTeam(AActor* Actor);

protected:
	UFUNCTION()
	void HandleForgotActor(AActor* Actor);

	UFUNCTION()
	void HandleSensedSight(AActor* Actor);

	UFUNCTION()
	void HandleLostSight(AActor* Actor);
	
	UFUNCTION()
	void HandleSensedSound(const FVector& Location);
	
	UFUNCTION()
	void HandleSensedDamage(AActor* Actor);

protected:
	
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& Actors);

public:
	UFUNCTION(BlueprintPure, Category="Enemy AI Controller")
	EAIState GetCurrentState();


	//////////////////////////////////////////////////////////////////////////
	// Blackboard Update Functions
	//////////////////////////////////////////////////////////////////////////
public:
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void BlackBoardUpdate_State(EAIState NewState);

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void BlackBoardUpdate_AttackRadius(float NewAttackRadius);

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void BlackBoardUpdate_DefendRadius(float NewDefendRadius);

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void BlackBoardUpdate_PointOfInterest(FVector NewPointOfInterest);

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void BlackBoardUpdate_AttackTarget(UObject* NewAttackTarget);


	//////////////////////////////////////////////////////////////////////////
	// Member Variables (Properties)
	//////////////////////////////////////////////////////////////////////////
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Enemy AI Controller")
	float TimeToSeekAfterLosingSight = 3.0f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Enemy AI Controller")
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Enemy AI Controller|Reference")
	TObjectPtr<AEnemyAIBase> AIPossessed;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Enemy AI Controller|Reference")
	TObjectPtr<UAIBehaviorsComponent> PossessedBehaviorsComp;

	UPROPERTY(BlueprintReadWrite, Category="Enemy AI Controller")
	AActor* AttackTarget;

	UPROPERTY(BlueprintReadOnly, Category="Enemy AI Controller")
	TArray<AActor*> KnownSeenActors;

private:
	// --- Internal Timer ---
	FTimerHandle CheckForgottenActorsTimer;

	FTimerHandle SeekAttackTargetTimer;
};
