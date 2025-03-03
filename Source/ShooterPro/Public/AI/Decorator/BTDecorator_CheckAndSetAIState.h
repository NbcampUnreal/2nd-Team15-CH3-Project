

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckAndSetAIState.generated.h"

/**
 * 데코레이터: 
 * - 현재 블랙보드에 있는 AI State(GameplayTag)를 확인하여, 원하는 상태와 같다면 `true` 반환.
 * - bDoStateUpdate == true 이면, 다를 경우 CanChangeState로 검사 후 상태를 업데이트 하고 결과 반환
 * - bDoStateUpdate == false 이면, 상태가 원하는 상태인지 단순 체크만 진행
 */
UCLASS()
class SHOOTERPRO_API UBTDecorator_CheckAndSetAIState : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UBTDecorator_CheckAndSetAIState();

protected:
	/**
	 * 이 데코레이터에서 원하는 AI State 태그
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Check AI State")
	FGameplayTag DesiredStateTag;

	/**
	 * false라면 체크만 하고, 다르면 false 반환
	 * true라면 체크 후, 다르면 CanChangeState -> UpdateState 시도
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Check AI State")
	bool bDoStateUpdate = false;

	/**
	 * 실제 조건 판단 함수. BT가 이 데코레이터 노드를 평가할 때 호출됩니다.
	 * @return true 이면 트리 진행, false 이면 데코레이터 실패 처리
	 */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;


};
