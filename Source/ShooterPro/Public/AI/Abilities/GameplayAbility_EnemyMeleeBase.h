#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProGameplayAbility.h"
#include "GameplayAbility_EnemyMeleeBase.generated.h"

/**
 * @brief 근접 공격 + 모션 워핑을 지원하는 AI 능력 클래스.
 *
 * - 공격 몽타주 재생 시, MotionWarpingComponent를 통해 지정된 여러 타겟 위치(또는 액터)로 이동을 보정 가능.
 * - 애니메이션 NotifyState_MotionWarping이 포함된 몽타주 사용 필수.
 * - 공격 이벤트 시점에 피해 적용. (기본 근접 공격 로직)
 * - BlueprintImplementableEvent로 SetupMotionWarpTarget를 열어둬, BP에서 원하는 방식으로 워프 타겟들 설정 가능.
 */
UCLASS()
class SHOOTERPRO_API UGameplayAbility_EnemyMeleeBase : public UProGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_EnemyMeleeBase();

protected:
	/** 
	 * @brief 능력 활성화 함수. 
	 *  1) 비용/쿨다운 커밋
	 *  2) 모션 워핑 타겟(들) 설정 (Blueprint 쪽에서 Setup 가능)
	 *  3) 공격 몽타주 재생 & 이벤트 처리
	 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	/** 
	 * @brief 능력 실행 중 몽타주가 중단(취소/인터럽트)되었을 때 
	 * 능력을 취소 처리
	 */
	UFUNCTION()
	void OnMontageInterruptedOrCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

	/** 
	 * @brief 몽타주가 정상적으로 종료되었을 때
	 * WarpTargetName(s) 제거 후 능력 정상 종료
	 */
	UFUNCTION()
	void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	/** 
	 * @brief 공격 이벤트 시점(AnimNotify → Gameplay Event)
	 * EventTag에 따라 (예: WarpEventTag, HitEventTag 등) 분기 가능
	 */
	UFUNCTION()
	void OnAttackEventReceived(FGameplayTag InEventTag, FGameplayEventData EventData);

	/**
	 * @brief MotionWarping용 타겟(들)을 설정하기 위한 함수 (Blueprint에서 구현 가능)
	 *
	 * 외부(BehaviorTree) 또는 AnimNotify에서 전달받은 InTargetActor 또는 기타 정보를 이용해
	 * BlueprintImplementableEvent로 여러 WarpTargetName에 대해서 AddOrUpdateWarpTargetFromTransform를 호출할 수 있음.
	 *
	 * @param InTargetActor 몬스터가 접근해야 할 대상(또는 위치를 알 수 있는 액터). nullptr 가능.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category="Melee|MotionWarp", meta=(DisplayName="Setup Motion Warp Targets"))
	void OnSetupMotionWarpTarget(const AActor* InTargetActor);

protected:
	/** 공격 몽타주 (RootMotion + AnimNotifyState_MotionWarping가 포함되어야 함) */
	UPROPERTY(EditDefaultsOnly, Category = "Melee|MotionWarp")
	TObjectPtr<UAnimMontage> AttackMontage;

	/** 몽타주 재생 속도 */
	UPROPERTY(EditDefaultsOnly, Category = "Melee|MotionWarp")
	float MontagePlayRate = 1.f;

	/** 
	 * @brief OnAttackEventReceived로 들어올 이벤트 태그 컨테이너.
	 * 예: {Event.Warp, Event.Hit} 등
	 */
	UPROPERTY(EditDefaultsOnly, Category="Melee|MotionWarp")
	FGameplayTagContainer EventTags;

	/** 워핑 시점 이벤트 예: "Event.Warp" */
	UPROPERTY(EditDefaultsOnly, Category="Melee|MotionWarp")
	FGameplayTag WarpEventTag;

	/** 실제 공격 시점 이벤트 예: "Event.Hit" */
	UPROPERTY(EditDefaultsOnly, Category="Melee|MotionWarp")
	FGameplayTag HitEventTag;

	/** 공격 시 적용할 EffectContainer용 태그 (HitEvent 시 사용) */
	UPROPERTY(EditDefaultsOnly, Category="Melee|MotionWarp")
	FGameplayTag AttackEffectTag;

	/**
	 * @brief 여러 개의 WarpTargetName을 지원하기 위해 배열화
	 * AnimNotifyState_MotionWarping에서 Name을 여러 개 운용할 수 있음.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Melee|MotionWarp")
	TArray<FName> WarpTargetNames;

	/** AI가 설정하는 타겟 액터 */
	UPROPERTY()
	TWeakObjectPtr<AActor> CachedTargetActor;
};
