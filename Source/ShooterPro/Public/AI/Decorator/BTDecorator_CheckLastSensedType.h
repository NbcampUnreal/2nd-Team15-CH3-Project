#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckLastSensedType.generated.h"

/**
 * @brief 최근 감지된 감각(LastSenseHandle.DetectedSense)이
 *        데코레이터에서 지정한 감각(열거형 EAISense)과 일치하는지 검사하는 데코레이터
 */
UCLASS()
class SHOOTERPRO_API UBTDecorator_CheckLastSensedType : public UBTDecorator
{
	GENERATED_BODY()

public:
	/** 생성자: 기본 노드 이름 설정 등 */
	UBTDecorator_CheckLastSensedType();

protected:
	/** BehaviorTree가 Condition을 평가할 때 호출되는 함수 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

#if WITH_EDITOR
	/** 에디터에서 노드를 볼 때 표시될 아이콘 */
	virtual FName GetNodeIconName() const override;
#endif

public:
	/** 비교할 감각(시야, 청각, 데미지 등)을 에디터에서 지정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sense")
	EAISense SenseToCheck = EAISense::Sight;
};
