
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckDistanceToTarget.generated.h"

/**
 * 데코레이터: 
 * - SelfActor와 TargetActor 간의 거리를 비교하고,
 * - 설정된 거리 값보다 작거나 같으면 true 반환.
 * - 불변수가 활성화되면 커스텀 거리 값을 사용하고, 그렇지 않으면 블랙보드에서 지정한 거리 값을 사용.
 */
UCLASS()
class SHOOTERPRO_API UBTDecorator_CheckDistanceToTarget : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_CheckDistanceToTarget();
	
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

	/** 셀프 액터와 타겟 액터 간의 거리를 비교 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

};
