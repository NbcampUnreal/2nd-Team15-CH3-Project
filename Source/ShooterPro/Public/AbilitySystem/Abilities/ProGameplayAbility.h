#pragma once

#include "CoreMinimal.h"
#include "Abilities/GSCGameplayAbility.h"
#include "ProGameplayAbility.generated.h"

class UProAbilityCondition;


/**
 * @brief UGSCGameplayAbility를 상속하여 'ProAbilityCondition'들을 통한 추가 조건 로직을 도입
 * - AdditionalConditions: 능력 활성화 전( `CheckCost` )에 조건을 검사하여 불충분 시 능력 발동 불가
 */
UCLASS()
class SHOOTERPRO_API UProGameplayAbility : public UGSCGameplayAbility
{
	GENERATED_BODY()

public:
	UProGameplayAbility();

protected:
	// ---------------------------------------------------------
	// UGameplayAbility Overrides
	// ---------------------------------------------------------
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
	                                FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;


	// ---------------------------------------------------------
	// Condition Logic
	// ---------------------------------------------------------

	/**
	 * @brief Ability를 활성화할 때 필요한 추가 조건들을 검사한다.
	 *
	 * 모든 UProAbilityCondition의 CheckCondition을 호출하여,
	 * 하나라도 실패하면 능력 발동 불가.
	 */
	virtual bool CheckAdditionalConditions(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                       FGameplayTagContainer* OptionalRelevantTags) const;

public:
	/**
	 * @brief 추가로 요구되는 '조건(Condition)' 목록
	 *
	 * 각 Condition 마다 CheckCondition을 호출해, 통과 못 하면 능력 활성화가 불가능하도록 처리
	 */
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Pro|AbilityCondition")
	TArray<TObjectPtr<UProAbilityCondition>> AdditionalConditions;
};
