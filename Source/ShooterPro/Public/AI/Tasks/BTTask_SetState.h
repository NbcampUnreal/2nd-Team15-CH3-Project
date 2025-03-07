#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetState.generated.h"

/**
 * @brief AI의 현재 상태를 설정하는 Task.
 * FGameplayTag를 받아 AI 상태를 업데이트합니다.
 */
UCLASS()
class SHOOTERPRO_API UBTTask_SetState : public UBTTaskNode
{
	GENERATED_BODY()

public:
	/** 기본 생성자 */
	UBTTask_SetState();

protected:
	/** 이 Task가 실행될 때 호출됩니다. */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	/** 설정할 AI 상태 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI|State")
	EAIState DesiredState;
};
