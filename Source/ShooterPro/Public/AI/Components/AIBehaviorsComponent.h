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
	bool UpdateState(FGameplayTag UpdateState);

	UFUNCTION(BlueprintPure, Category="AI Behavior")
	bool IsInCombat();

	UFUNCTION(BlueprintPure, Category="AI Behavior")
	const FPerceivedActorInfo& GetLastSenseHandle() { return RecentSenseHandle; }

	UFUNCTION(BlueprintCallable,Category="AI Behavior")
	void ForceAttackTarget(AActor* NewActor);

public:
	UFUNCTION()
	void HandlePerceptionUpdated(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandlePerceptionForgotten(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void RemoveActorFromAttackList(AActor* LostActor);

public:
	UFUNCTION()
	void HandleSensedSight();
	


	UFUNCTION()
	void HandleSensedSound();

	UFUNCTION()
	void HandleSensedDamage();

		UFUNCTION()
    	void HandleLostSight();
    	
	UFUNCTION()
	void HandleLostSound();

	UFUNCTION()
	void HandleLostDamage();

public:
	UFUNCTION()
	void SetStateAsAttacking();

	UFUNCTION()
	void SetStateAsSeeking();

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
	float WalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float JogSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float SprintingSpeed = 600.0f;

	/** 시야에서 벗어난 후, 몇 초 뒤에 타겟을 제거할 것인지 */
	UPROPERTY(EditAnywhere, Category="AI Behavior|Config")
	float ForgetSightTime = 3.0f;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Combat Trigger")
	TMap<EAISense, float> AISensePriority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Combat Trigger", meta=(Bitmask, BitmaskEnum="ECombatTriggerFlags"))
	int32 CombatTriggerMask = static_cast<uint8>(ECombatTriggerFlags::Sight);

public:
	UPROPERTY(BlueprintReadWrite, Category="AI Behavior|Combat")
	AActor* AttackTarget;

	//공격 가능한 액터들
	UPROPERTY(BlueprintReadOnly, Category="AI Behavior|Combat")
	TArray<AActor*> AttackableTargets;

public:
	// // 이전 AI 상태
	// UPROPERTY(BlueprintReadOnly, Category="AI Behavior")
	// FGameplayTag PreviousState;
	//
	// // 현재 AI 상태
	// UPROPERTY(BlueprintReadOnly, Category="AI Behavior")
	// FGameplayTag CurrentState;

private:
	//공격 거리
	UPROPERTY(EditAnywhere, Category="AI Behavior")
	float AttackRadius = 200.0f;

	//방어 거리
	UPROPERTY(EditAnywhere, Category="AI Behavior")
	float DefendRadius = 400.0f;

	//랜덤 거리
	UPROPERTY(EditAnywhere, Category="AI Behavior")
	float MaxRandRadius = 500.0f;

private:
	//최근 감지한 정보 구조체
	UPROPERTY(BlueprintReadOnly, Category="AI Behavior", meta=(AllowPrivateAccess=true))
	FPerceivedActorInfo RecentSenseHandle;

private:
	/** '시야를 벗어난 타겟'을 지우기 위한 타이머를 저장하는 맵 */
	UPROPERTY()
	TMap<AActor*, FTimerHandle> ForgetTimers;


	// FTimerHandle SeekTimerHandle;
};



/* 옛날 코드들
	public:
	UFUNCTION(BlueprintPure, Category="AI Behavior|Movement Setting")
	float GetRealRotationRate();

	float UAIBehaviorsComponent::GetRealRotationRate()
	{
		return CurrentState == AIGameplayTags::AIState_Combat ? CombatRotationRate : InitialRotationRate;
	}


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	bool bUseAimOffset = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float InitialRotationRate = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|Config")
	float CombatRotationRate = 270.0f;
 */
