#pragma once

#include "CoreMinimal.h"
#include "AI/AIDectionInfoTypes.h"
#include "AI/EnemyAITypes.h"
#include "Components/ActorComponent.h"
#include "ProAIBehaviorsComponent.generated.h"

class AEnemyAIBase;
class AEnemyAIController;
class AAIController;

/**
 * AI 행동/상태 로직 관리용 컴포넌트
 */
UCLASS(ClassGroup=("AI"), meta=(BlueprintSpawnableComponent))
class SHOOTERPRO_API UProAIBehaviorsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UProAIBehaviorsComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void InitializeBehavior(class AEnemyAIController* PossedController);


public:
	// 상태 전이 가능 여부 체크 함수
	UFUNCTION(BlueprintCallable, Category="AI Behavior")
	bool CanChangeState(EAIState ChangeState);

	// 상태 전이(실제 업데이트) 함수
	UFUNCTION(BlueprintCallable, Category="AI Behavior")
	bool UpdateState(EAIState UpdateState);
	

	/*=======================================
	  * 전용 함수 (CanChangeStateToXXX, HandleEnterXXXState 등)
	  *======================================*/
private:
	// 상태 전이 가능 여부 판단용
	bool CanChangeStateToDead() const;
	bool CanChangeStateToCombat() const;
	bool CanChangeStateToIdle() const;
	bool CanChangeStateToSeeking() const;
	bool CanChangeStateToDisabled() const;

	// 상태 전이 후(진입 시) 해야 할 작업들
	void HandleEnterDeadState();
	void HandleEnterCombatState();
	void HandleEnterIdleState();
	void HandleEnterSeekingState();
	void HandleEnterDisabledState();

public:
	UFUNCTION(BlueprintCallable, Category="AI Behavior")
	void ForceAttackTarget(AActor* NewActor);

protected:
	// 지각 관련 처리
	UFUNCTION()
	void HandlePerceptionUpdated(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void HandlePerceptionForgotten(const FPerceivedActorInfo& PerceivedActorInfo);

	UFUNCTION()
	void RemoveActorFromAttackList(AActor* LostActor);

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
	// 외부에서 호출 가능한 “스테이트 설정” 함수(상태 세팅 내부 처리)
	UFUNCTION()
	void SetStateAsAttacking();

	UFUNCTION()
	void SetStateAsSeeking();

	// Getters
public:
	UFUNCTION(BlueprintPure, Category="AI Behavior|Getter")
	float GetAttackRadius() const { return AttackRadius; }

	UFUNCTION(BlueprintPure, Category="AI Behavior|Getter")
	float GetDefendRadius() const { return DefendRadius; }

	UFUNCTION(BlueprintPure, Category="AI Behavior|Getter")
	float GetMaxRandRadius() const { return MaxRandRadius; }

	UFUNCTION(BlueprintPure, Category="AI Behavior|Getter")
	bool IsInCombat();

	UFUNCTION(BlueprintPure, Category="AI Behavior|Getter")
	const FPerceivedActorInfo& GetLastSenseHandle() { return RecentSenseHandle; }

	// 트리거 활성화 여부 체크
	UFUNCTION(BlueprintPure, Category="AI Behavior|Getter")
	bool IsTriggerEnabled(ECombatTriggerFlags Trigger) const;

protected:
	UPROPERTY(BlueprintReadWrite, Category="AI Behavior")
	TObjectPtr<AEnemyAIBase> CharacterRef;

	UPROPERTY(BlueprintReadWrite, Category="AI Behavior")
	TObjectPtr<AEnemyAIController> AIControllerRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behavior|State")
	float TimeToSeekAfterLosingSight = 3.0f;

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
};
