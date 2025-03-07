// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/BTDecorator_CheckPreviousAndCurrentState.h"

#include "AIController.h"
#include "AI/EnemyAIController.h"
#include "AI/Components/ProAIBehaviorsComponent.h"

UBTDecorator_CheckPreviousAndCurrentState::UBTDecorator_CheckPreviousAndCurrentState()
{
	NodeName = TEXT("Check Previous & Current State");
	bNotifyBecomeRelevant = false;
	bNotifyCeaseRelevant = false;
}


/**
 * @brief 데코레이터의 핵심: 이전 상태와 현재 상태가 각각 원하는 GameplayTag인지 검사
 */
bool UBTDecorator_CheckPreviousAndCurrentState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	// 기본값(조건 불만족)
	bool bConditionResult = true;

	// 1. AAIController 가져오기
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return false;
	}

	// 2. Pawn에서 UAIBehaviorsComponent 찾기
	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return false;
	}
	
	// 3. 이전 상태 검사
	if (bCheckPreviousState)
	{
		if (AIController->GetPreviousState() != CheckPreviousState)
		{
			// 이전 상태가 다르면 false
			bConditionResult = false;
		}
	}

	// 4. 현재 상태 검사
	if (bCheckCurrentState)
	{
		if (AIController->GetCurrentState() != CheckCurrentState)
		{
			// 현재 상태가 다르면 false
			bConditionResult = false;
		}
	}
	
	if (!bCheckPreviousState && !bCheckCurrentState)
	{
		bConditionResult = true;
	}

	return bConditionResult;
}
