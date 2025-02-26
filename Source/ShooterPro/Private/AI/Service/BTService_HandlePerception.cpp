#include "AI/Service/BTService_HandlePerception.h"

#include "AI/EnemyAIController.h"
#include "AI/EnemyAILog.h"
#include "AI/Components/AIBehaviorsComponent.h"
#include "Perception/AIPerceptionComponent.h"

UBTService_HandlePerception::UBTService_HandlePerception()
{
	// BTService 기본 설정
	Interval = 0.5f; // 0.5초마다 실행
	RandomDeviation = 0.1f; // ±0.1초의 랜덤 편차
	bNotifyTick = true;
	bNotifyOnSearch = false;
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
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
			// CachedController->GetDetectionInfoManager()->OnAddPerceptionUpdated.AddDynamic(this, &UBTService_HandlePerception::OnPerceptionUpdated);
			// CachedController->GetDetectionInfoManager()->OnRemoveExpiredDetection.AddDynamic(this, &UBTService_HandlePerception::OnTargetPerceptionForgotten);

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

	// CachedController->GetDetectionInfoManager()->OnAddPerceptionUpdated.RemoveDynamic(this, &UBTService_HandlePerception::OnPerceptionUpdated);
	// CachedController->GetDetectionInfoManager()->OnRemoveExpiredDetection.RemoveDynamic(this, &UBTService_HandlePerception::OnTargetPerceptionForgotten);
}

void UBTService_HandlePerception::OnPerceptionUpdated(const FPerceivedActorInfo& PerceivedActorInfo)
{
	if (UAIBehaviorsComponent* AIBehaviorsComponent = CachedBehaviorsComp.Get())
	{
		AIBehaviorsComponent->HandlePerceptionUpdated(PerceivedActorInfo);
	}
}

void UBTService_HandlePerception::OnTargetPerceptionForgotten(const FPerceivedActorInfo& PerceivedActorInfo)
{
	if (UAIBehaviorsComponent* AIBehaviorsComponent = CachedBehaviorsComp.Get())
	{
		AIBehaviorsComponent->HandlePerceptionForgotten(PerceivedActorInfo);
	}
}
