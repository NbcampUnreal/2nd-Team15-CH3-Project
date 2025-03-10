#include "AbilitySystem/Abilities/ProGameplayAbility.h"

#include "ProGmaeplayTag.h"
#include "AbilitySystem/AbilityTypes.h"
#include "AbilitySystem/Abilities/ProAbilityCondition.h"
#include "AbilitySystem/Abilities/ProAbilityCost.h"
#include "GameFramework/GameplayMessageSubsystem.h"


UProGameplayAbility::UProGameplayAbility()
{
}

bool UProGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                             const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                             FGameplayTagContainer* OptionalRelevantTags) const
{
	// 1) 부모 로직 (쿨다운, 태그 블록 등 GAS 전반 검사)
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 2) 추가 조건 검사
	//    예: 무기 장착 여부, 팀 체크, 자세/상태, 기타 기획요소 등
	if (!CheckAdditionalConditions(Handle, ActorInfo, OptionalRelevantTags))
	{
		// CheckAdditionalConditions가 false면
		// OptionalRelevantTags에 실패 태그를 더 넣어 UI 등에 피드백할 수 있음
		return false;
	}

	return true; // 전부 통과하면 OK
}

bool UProGameplayAbility::CheckAdditionalConditions(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                    FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	// 모든 AdditionalConditions를 순회하며 CheckCondition 실행
	for (UProAbilityCondition* Condition : AdditionalConditions)
	{
		if (!Condition)
		{
			continue;
		}

		const bool bPassed = Condition->CheckCondition(this, Handle, ActorInfo, OptionalRelevantTags);
		if (!bPassed)
		{
			return false; // 하나라도 실패하면 능력 발동 불가능
		}
	}

	return true; // 전부 통과하면 true
}

bool UProGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags)) return false;

	for (const TObjectPtr<UProAbilityCost>& AdditionalCost : ExtendedCosts)
	{
		if (AdditionalCost != nullptr)
		{
			// UProAbilityConst를 상속받은 클래스는 CheckCost를 구현하도록.
			if (!AdditionalCost->CheckCost(this, Handle, ActorInfo, OptionalRelevantTags))
			{
				return false;
			}
		}
	}

	return true;
}

void UProGameplayAbility::ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                    const FGameplayAbilityActivationInfo ActivationInfo) const
{
	Super::ApplyCost(Handle, ActorInfo, ActivationInfo);

	check(ActorInfo);

	for (const TObjectPtr<UProAbilityCost>& AdditionalCost : ExtendedCosts)
	{
		if (AdditionalCost != nullptr)
		{
			// UProAbilityConst를 상속받은 클래스는 ApplyCost를 구현하도록.
			AdditionalCost->ApplyCost(this, Handle, ActorInfo, ActivationInfo);
		}
	}
}
