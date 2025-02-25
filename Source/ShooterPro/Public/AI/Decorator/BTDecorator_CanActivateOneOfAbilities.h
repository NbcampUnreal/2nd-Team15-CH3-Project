
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanActivateOneOfAbilities.generated.h"

/**
 * 데코레이터:
 *   - AbilityTags 안에 들어있는 임의의 태그를 가진 능력(Ability)이
 *   - 현재 CanActivateAbility()로 활성화 가능 상태인지 검사.
 *   - 하나라도 가능하면 true, 전부 불가능하면 false
 */
UCLASS()
class SHOOTERPRO_API UBTDecorator_CanActivateOneOfAbilities : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CanActivateOneOfAbilities();

protected:
	/**
	 * @brief Behavior Tree가 이 데코레이터의 조건을 평가할 때 호출되는 함수
	 * @param OwnerComp  BT 컴포넌트
	 * @param NodeMemory 노드 메모리
	 * @return 조건이 참이면 true, 거짓이면 false
	 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif

public:
	/**
	 * AbilityTags 컨테이너에 들어있는 태그들 중
	 * 하나라도 활성화 가능한 Ability가 있다면 true를 반환합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI|Ability")
	FGameplayTagContainer AbilityTags;
};
