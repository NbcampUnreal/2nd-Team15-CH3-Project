#include "AI/Service/BTService_HandlePerception.h"

#include "AI/EnemyAIController.h"
#include "AI/EnemyAILog.h"
#include "AI/Components/ProAIBehaviorsComponent.h"


UBTService_HandlePerception::UBTService_HandlePerception()
{
	Interval = 0.5f; // 0.5초마다 실행
	RandomDeviation = 0.1f; // ±0.1초의 랜덤 편차
	bNotifyTick = true;
	bNotifyOnSearch = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	bCreateNodeInstance = true; // 이 값을 true로 설정하면 각 AI마다 독립된 서비스 인스턴스를 만듭니다.
}

void UBTService_HandlePerception::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	// AIController 캐스팅 (매 Tick마다 찾는 것은 낭비이므로 한 번만 해둠)
	if (AAIController* AICon = OwnerComp.GetAIOwner())
	{
		CachedController = Cast<AEnemyAIController>(AICon);
		if (CachedController.IsValid())
		{
			CachedBehaviorsComp = CachedController->AIBehaviorComponent;
		}
	}
}

void UBTService_HandlePerception::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// Tick마다 Check (만약 포인터 만료/Invalid 되었다면 다시 캐스팅 필요할 수도)
	if (AEnemyAIController* EnemyAIController = CachedController.Get())
	{
		EnemyAIController->UpdatePerception(Interval);
	}
}

void UBTService_HandlePerception::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
}

