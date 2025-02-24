// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/BTDecorator_CheckDistanceToTarget.h"

#include "AI/EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTDecorator_CheckDistanceToTarget::UBTDecorator_CheckDistanceToTarget()
{
	NodeName = TEXT("Check Distance to Target");

	// 기본 값 설정
	bUseCustomDistance = false;
	CustomDistance = 500.0f;
}

bool UBTDecorator_CheckDistanceToTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// 1) AI 컨트롤러 가져오기
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return false;
	}

	// 2) 블랙보드에서 타겟 액터 가져오기
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return false;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		return false;
	}

	// 3) 셀프 액터 가져오기
	AActor* SelfActor = AIController->GetPawn();
	if (!SelfActor)
	{
		return false;
	}

	// 4) 거리 계산 (커스텀 거리 사용 여부 판단)
	float DistanceToTarget = FVector::Dist(SelfActor->GetActorLocation(), TargetActor->GetActorLocation());
	float DistanceThreshold = bUseCustomDistance ? CustomDistance : BlackboardComp->GetValueAsFloat(RangeKey.SelectedKeyName);

	// 5) 거리가 임계값보다 작거나 같으면 true 반환
	return DistanceToTarget <= DistanceThreshold;
}
