// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_SetState.h"

#include "AI/EnemyAIController.h"
#include "AI/Components/AIBehaviorsComponent.h"


UBTTask_SetState::UBTTask_SetState()
{
	// NodeName을 설정해주면 Behavior Tree에서 해당 노드를 찾을 수 있음
	NodeName = TEXT("Set AI State");
}

EBTNodeResult::Type UBTTask_SetState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 1. AIController와 AIBehaviorsComponent를 가져옵니다.
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UAIBehaviorsComponent* BehaviorsComp = AIController->AIBehaviorComponent;
	if (!BehaviorsComp)
	{
		return EBTNodeResult::Failed;
	}

	// 2. 상태 업데이트
	BehaviorsComp->UpdateState(DesiredStateTag);

	// 3. 성공적으로 상태를 설정했으므로 Task가 끝났다고 반환
	return EBTNodeResult::Succeeded;
}
