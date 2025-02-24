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
		CachedAIController = Cast<AEnemyAIController>(AICon);
		CachedAIBehaviorsComp = CachedAIController->GetPawn()->FindComponentByClass<UAIBehaviorsComponent>();
	}
}

void UBTService_SelectAttackTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 공격 가능한 타겟을 선택
	if (CachedAIBehaviorsComp.IsValid())
	{
		SelectAttackTarget(OwnerComp);
	}
}

void UBTService_SelectAttackTarget::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	// 관련 없는 경우에는 캐시된 컴포넌트 지우기
	CachedAIBehaviorsComp.Reset();
	CachedAIController.Reset();
}

void UBTService_SelectAttackTarget::SelectAttackTarget(UBehaviorTreeComponent& OwnerComp)
{
	// 공격 가능한 타겟 목록을 가져옵니다.
	const TArray<AActor*>& AttackableTargets = CachedAIBehaviorsComp->AttackableTargets;

	// 타겟이 없다면, 리턴
	if (AttackableTargets.IsEmpty())
	{
		CachedAIController->UpdateBlackboard_AttackTarget(nullptr);
		CachedAIBehaviorsComp->AttackTarget = nullptr;
		return;
	}

	// 가장 가까운 타겟을 선택
	AActor* BestTarget = nullptr;
	float ShortestDistance = FLT_MAX;

	// 기본적으로는 거리 기반으로 가장 가까운 타겟을 선택
	for (AActor* PotentialTarget : AttackableTargets)
	{
		if (!IsValid(PotentialTarget))
			continue;

		//ToDo::나중에 죽음에 대한 것도 처리해줘야합니다.

		float Distance = FVector::Dist(CachedAIBehaviorsComp->GetOwner()->GetActorLocation(), PotentialTarget->GetActorLocation());

		// 가장 가까운 타겟을 선택
		if (Distance < ShortestDistance)
		{
			ShortestDistance = Distance;
			BestTarget = PotentialTarget;
		}
	}

	// 최적의 공격 대상을 설정
	CachedAIBehaviorsComp->AttackTarget = BestTarget;
	CachedAIController->UpdateBlackboard_AttackTarget(BestTarget);
}
