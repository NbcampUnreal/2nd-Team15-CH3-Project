
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_BlackboardBase.h"
#include "BTDecorator_CheckDistanceToTarget.generated.h"


/**
 * 데코레이터:
 *  - Player(또는 TargetActor)와의 거리를 블랙보드 상에서 확인
 *  - 일정 거리 이하인지(또는 이상인지) 판별
 *  - FlowAbortMode & OnBlackboardKeyValueChange 로직을 통해 즉시 Abort/재실행
 */
UCLASS()
class SHOOTERPRO_API UBTDecorator_CheckDistanceToTarget : public UBTDecorator_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckDistanceToTarget();

protected:
	
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	/** 셀프 액터와 타겟 액터 간의 거리를 비교 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	EBlackboardNotificationResult OnBBValueChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key, UBehaviorTreeComponent* BehaviorTreeComponent, unsigned char* Arg);
	
protected:
	/** 블랙보드에서 가져올 액터의 키 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Blackboard")
	FBlackboardKeySelector RangeKey;
	
	
	/** 커스텀 거리 값 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Distance")
	bool bUseCustomDistance = false;

	/** 커스텀 거리 값 (불변수가 활성화되면 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Distance",meta=(EditCondition="bUseCustomDistance"))
	float CustomDistance = 500.0f;



};
