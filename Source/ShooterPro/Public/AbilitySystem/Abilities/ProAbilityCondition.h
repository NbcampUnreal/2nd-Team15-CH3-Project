#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "UObject/Object.h"
#include "ProAbilityCondition.generated.h"

class UProGameplayAbility;


/**
 * @brief ProAbilityCondition
 *
 * - 능력(Ability)을 활성화하기 전, 추가로 확인해야 할 조건(상태, 상황, etc)을 정의.
 * - 예: 특정 무기 장착 여부, 현재 에너지 값이 일정 이상인지, 특정 태그 상태인지 등
 */
UCLASS(Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew, Abstract)
class SHOOTERPRO_API UProAbilityCondition : public UObject
{
	GENERATED_BODY()

public:
	UProAbilityCondition()
	{
	}

	/**
	  * @brief 실제로 조건을 검사하는 함수
	  *
	  * @param Ability   : 체크하려는 Ability (UProGameplayAbility)
	  * @param Handle    : AbilitySpecHandle
	  * @param ActorInfo : Ability를 실행하는 액터 정보
	  * @param OptionalRelevantTags : 조건 실패 시 태그를 추가할 수도 있음 (UI 피드백용)
	  *
	  * @return true면 조건 충족(능력 사용 가능), false면 조건 불충족
	  */
	virtual bool CheckCondition(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo* ActorInfo,
								FGameplayTagContainer* OptionalRelevantTags) const
	{
		return K2_CheckCondition(Ability, Handle, *ActorInfo, *OptionalRelevantTags); // 기본은 무조건 통과
	}
	
	UFUNCTION(BlueprintImplementableEvent)
	bool K2_CheckCondition(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo& ActorInfo,
								FGameplayTagContainer& OptionalRelevantTags) const;
};
