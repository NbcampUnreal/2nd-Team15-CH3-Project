// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/BTDecorator_CheckAndSetAIState.h"

#include "AI/EnemyAIController.h"
#include "AI/EnemyAILog.h"
#include "AI/Components/ProAIBehaviorsComponent.h"
#include "AI/Utility/EnemyAIBluePrintFunctionLibrary.h"


UBTDecorator_CheckAndSetAIState::UBTDecorator_CheckAndSetAIState()
{
	NodeName = TEXT("Check & (Maybe) Set AIState");

	// 데코레이터가 ConditionOnly 모드로 동작하도록 기본 설정
	bNotifyBecomeRelevant = false;
	bNotifyCeaseRelevant = false;
}

bool UBTDecorator_CheckAndSetAIState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	// 1) AI 컨트롤러 가져오기
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return false;
	}

	// 2) 블랙보드에서 현재 AI State(FGameplayTag)를 가져옴
	EAIState CurrentState = AIController->GetCurrentState();
	if (CurrentState == DesiredState)
	{
		// 이미 원하는 태그를 가지고 있다면 TRUE
		return true;
	}

	// ---- bDoStateUpdate == false -> 단순 체크만 수행 ----
	if (!bDoStateUpdate)
	{
		// 상태가 DesiredStateTag와 다르면 false
		return false;
	}

	// ---- bDoStateUpdate == true -> CanChangeState 후 UpdateState ----
	UProAIBehaviorsComponent* AIBehaviors = AIController->AIBehaviorComponent;
	if (!AIBehaviors)
	{
		return false;
	}

	if (AIBehaviors->CanChangeState(DesiredState))
	{
		return AIBehaviors->UpdateState(DesiredState);
	}

	// 변경 불가하면 false
	return false;
}
