#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProAbilityCondition.h"
#include "ProAbilityCondition_CheckAttackDistance.generated.h"

/**
 * @brief 공격 대상(AttackTarget)과의 거리를 검사하는 Condition 클래스
 *
 * - UAIBehaviorsComponent 의 AttackTarget와 아바타(AI) 간 거리를 측정
 * - Distance <= MaxDistance 이면 true 반환
 * - AttackTarget가 없으면 false 반환
 */
UCLASS()
class SHOOTERPRO_API UProAbilityCondition_CheckAttackDistance : public UProAbilityCondition
{
	GENERATED_BODY()

public:
	UProAbilityCondition_CheckAttackDistance();

protected:
	/**
	 * @brief Condition 검사 로직
	 *
	 * @param Ability   : 검사 대상 Ability
	 * @param Handle    : AbilitySpecHandle
	 * @param ActorInfo : ASC ActorInfo
	 * @param OptionalRelevantTags : 실패 시 태그를 추가할 수 있는 컨테이너
	 * @return true면 조건 충족(AttackTarget와 거리 <= MaxDistance), false면 불충족
	 */
	virtual bool CheckCondition(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle,
	                            const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;

public:
	/** 거리가 이 값 이하이면 조건 충족 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Condition|AttackDistance")
	float MaxDistance = 200.0f;
};
