#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CanUseGameplayAbility.generated.h"

/**
 * BT 데코레이터: 특정 GameplayTag와 연결된 Ability가 사용 가능한지 판별
 */
UCLASS()
class SHOOTERPRO_API UBTDecorator_CanUseGameplayAbility : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CanUseGameplayAbility();

protected:
	/** Behavior Tree 에서 Condition이 참인지 계산한다 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

#if WITH_EDITOR
	/** 에디터에서 노드의 텍스트를 보여줄 때 표시할 간단한 설명 */
	virtual FName GetNodeIconName() const override;
#endif

public:
	/** 사용 가능 여부를 판별하고 싶은 Ability를 식별하는 GameplayTag */
	UPROPERTY(EditAnywhere, Category="AI|Ability")
	FGameplayTag AbilityTag;

	/**
	 * 만약 블랙보드에 있는 액터(혹은 폰)에서 Ability를 검사해야 한다면  
	 * 블랙보드 키도 같이 받아올 수 있습니다.
	 */
	UPROPERTY(EditAnywhere, Category="AI|Blackboard")
	struct FBlackboardKeySelector TargetActorKey;
};
