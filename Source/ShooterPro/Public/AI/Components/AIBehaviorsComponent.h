#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AI/AIDectionInfoTypes.h"
#include "AI/EnemyAITypes.h"
#include "Components/ActorComponent.h"
#include "AIBehaviorsComponent.generated.h"


class AEnemyAIBase;
class AEnemyAIController;
class AAIController;

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
	bool IsTriggerEnabled(ECombatTriggerFlags Trigger) const;

	UFUNCTION(BlueprintCallable, Category="AI Behavior")
	bool CanChangeState(FGameplayTag ChangeState);

	UFUNCTION(BlueprintCallable, Category="AI Behavior")
	void UpdateState(FGameplayTag UpdateState);

	UFUNCTION(BlueprintPure, Category="AI Behavior")
	bool IsInCombat();

	UFUNCTION(BlueprintPure, Category="AI Behavior")
	const FPerceivedActorInfo& GetLastSenseHandle() { return LastSenseHandle; }

public:
	UFUNCTION()
	void HandlePerceptionUpdated(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandlePerceptionForgotten(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandleForgotActor(const FPerceivedActorInfo& PerceivedActorInfo);

public:
	UFUNCTION()
	void HandleSensedSight(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandleLostSight(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandleSensedSound(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandleSensedDamage(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandleLostSound(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandleLostDamage(const FPerceivedActorInfo& PerceivedActorInfo);

public:
	
	UFUNCTION()
	void SetStateAsAttacking();

	UFUNCTION()
	void SetStateAsSeeking();

public:
	UFUNCTION(BlueprintPure, Category="AI Behavior|Movement Setting")
	float GetRealRotationRate();

public:
	UFUNCTION(BlueprintPure, Category="AI Behavior")
	float GetAttackRadius() const { return AttackRadius; }

	UFUNCTION(BlueprintPure, Category="AI Behavior")
	float GetDefendRadius() const { return DefendRadius; }

	UFUNCTION(BlueprintPure, Category="AI Behavior")
	float GetMaxRandRadius() const { return MaxRandRadius; }

protected:
	UPROPERTY(BlueprintReadWrite, Category="AI Behavior")
	TObjectPtr<AEnemyAIBase> CharacterRef;

	UPROPERTY(BlueprintReadWrite, Category="AI Behavior")
	TObjectPtr<AEnemyAIController> AIControllerRef;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|State")
	float TimeToSeekAfterLosingSight = 3.0f;

	// UPROPERTY(BlueprintReadWrite, Category="AI Behavior")
	// bool bInCombat = false;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	bool bUseAimOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float InitialRotationRate = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float CombatRotationRate = 270.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float WalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float JogSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float SprintingSpeed = 600.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Combat Trigger")
	TMap<EAISense, float> AISensePriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Combat Trigger", meta=(Bitmask, BitmaskEnum="ECombatTriggerFlags"))
	int32 CombatTriggerMask = static_cast<uint8>(ECombatTriggerFlags::Sight);

public:
	UPROPERTY(BlueprintReadWrite, Category="AI Behavior|Combat")
	AActor* AttackTarget;

	UPROPERTY(BlueprintReadOnly, Category="AI Behavior|Combat")
	TArray<AActor*> AttackableTargets;

public:
	// 이전 AI 상태
	UPROPERTY(BlueprintReadOnly, Category="AI Behavior")
	FGameplayTag PreviousState;

	// 현재 AI 상태
	UPROPERTY(BlueprintReadOnly, Category="AI Behavior")
	FGameplayTag CurrentState;

private:
	UPROPERTY(EditAnywhere, Category="AI Behavior")
	float AttackRadius = 200.0f;

	UPROPERTY(EditAnywhere, Category="AI Behavior")
	float DefendRadius = 400.0f;

	UPROPERTY(EditAnywhere, Category="AI Behavior")
	float MaxRandRadius = 500.0f;

private:
	UPROPERTY(BlueprintReadOnly, Category="AI Behavior", meta=(AllowPrivateAccess=true))
	FPerceivedActorInfo LastSenseHandle;

	// FTimerHandle SeekTimerHandle;
};
