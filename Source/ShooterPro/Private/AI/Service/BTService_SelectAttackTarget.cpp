// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_SelectAttackTarget.h"

#include "AIController.h"
#include "AI/EnemyAIController.h"
#include "AI/Components/AIBehaviorsComponent.h"


UBTService_SelectAttackTarget::UBTService_SelectAttackTarget()
{
	// 기본 설정
	Interval = 0.25f; // 1초마다 실행
	RandomDeviation = 0.1f; // ±0.1초 랜덤 편차
	bNotifyTick = true;
	bNotifyOnSearch = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_SelectAttackTarget::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	// AIBehaviorsComponent 캐시
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		AEnemyAIController* AIController  = Cast<AEnemyAIController>(AICon);
		CachedAIController.AddUnique(AIController);
		UAIBehaviorsComponent* AIBehaviorsComponent = AIController->GetPawn()->FindComponentByClass<UAIBehaviorsComponent>();
		CachedAIBehaviorsComp.AddUnique(AIBehaviorsComponent);
	}
}

void UBTService_SelectAttackTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	for (TWeakObjectPtr<UAIBehaviorsComponent> UaiBehaviorsComponent : CachedAIBehaviorsComp)
	{
		if (UaiBehaviorsComponent.IsValid())
		{
			UaiBehaviorsComponent->SetStateAsAttacking();
		}
	}		
}

void UBTService_SelectAttackTarget::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	// 관련 없는 경우에는 캐시된 컴포넌트 지우기
	CachedAIBehaviorsComp.Reset();
	CachedAIController.Reset();
}

