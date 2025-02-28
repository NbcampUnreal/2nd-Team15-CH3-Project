#include "AI/Service/BTService_SelectAttackTarget.h"
#include "AIController.h"
#include "AI/EnemyAIController.h"
#include "AI/EnemyAILog.h"
#include "AI/Components/AIBehaviorsComponent.h"


UBTService_SelectAttackTarget::UBTService_SelectAttackTarget()
{
	Interval = 0.25f;
	RandomDeviation = 0.1f;
	bNotifyTick = true;
	bNotifyOnSearch = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	bCreateNodeInstance = true; // 인스턴싱
}

void UBTService_SelectAttackTarget::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		CachedAIControllers = Cast<AEnemyAIController>(AICon);;
		CachedAIBehaviorsComps = CachedAIControllers->GetPawn()->FindComponentByClass<UAIBehaviorsComponent>();
	}
}

void UBTService_SelectAttackTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (CachedAIBehaviorsComps.IsValid())
	{
		CachedAIBehaviorsComps->SetStateAsAttacking();
	}
}

void UBTService_SelectAttackTarget::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	CachedAIBehaviorsComps.Reset();
	CachedAIControllers.Reset();
}
