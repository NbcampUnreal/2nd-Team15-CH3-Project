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

	// Blackboard 키가 바뀌거나, 조건식이 바뀌면
	// 우선순위가 낮은 브랜치들을 중단(Abort)하도록 설정
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;

	
	// 블랙보드 Key 필터 등록(중요!)
	//  -> 이 키들이 변하면, 엔진이 이 Decorator를 재평가 대상으로 삼음
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckDistanceToTarget, TargetActorKey), AActor::StaticClass());
	RangeKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckDistanceToTarget, RangeKey));

	// OnBecomeRelevant, OnCeaseRelevant 등을 사용하려면 아래 3개 플래그가 true여야 함
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant  = true;
	// bNotifyTick = false; // 필요하면 true로
}

void UBTDecorator_CheckDistanceToTarget::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);

	if (UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent())
	{
		// TargetActorKey와 RangeKey에 대해 값이 바뀔 때마다 OnBBValueChanged 콜백을 받도록 옵저버 등록
		{
			FOnBlackboardChangeNotification BBDelegate;
			BBDelegate.BindUObject(this, &UBTDecorator_CheckDistanceToTarget::OnBBValueChanged, &OwnerComp, NodeMemory);
			BBComp->RegisterObserver(RangeKey.GetSelectedKeyID(), this, BBDelegate);
		}
		{
			FOnBlackboardChangeNotification BBDelegate2;
			BBDelegate2.BindUObject(this, &UBTDecorator_CheckDistanceToTarget::OnBBValueChanged, &OwnerComp, NodeMemory);
			BBComp->RegisterObserver(TargetActorKey.GetSelectedKeyID(), this, BBDelegate2);
		}
	}
}

void UBTDecorator_CheckDistanceToTarget::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);

	if (UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent())
	{
		// 이 Decorator(=this)에 등록된 옵저버를 전부 해제
		BBComp->UnregisterObserversFrom(this);
	}
}

bool UBTDecorator_CheckDistanceToTarget::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIController) return false;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return false;

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor) return false;

	AActor* SelfActor = AIController->GetPawn();
	if (!SelfActor) return false;

	const float DistanceToTarget = FVector::Dist(SelfActor->GetActorLocation(), TargetActor->GetActorLocation());
	const float DistanceThreshold = bUseCustomDistance ? CustomDistance : BlackboardComp->GetValueAsFloat(RangeKey.SelectedKeyName);

	// 조건: 거리 ≤ 임계값
	return (DistanceToTarget <= DistanceThreshold);
}

EBlackboardNotificationResult UBTDecorator_CheckDistanceToTarget::OnBBValueChanged(const UBlackboardComponent& BlackboardComponent, FBlackboard::FKey Key, UBehaviorTreeComponent* BehaviorTreeComponent, unsigned char* Arg)
{
	// 안전 체크
	if (!BehaviorTreeComponent || !Arg)
	{
		return EBlackboardNotificationResult::ContinueObserving;
	}

	// Decorator 메모리
	uint8* NodeMemory = static_cast<uint8*>(Arg);

	// --- 핵심 변경 ---
	// Decorator가 "값이 바뀌었다"는 것을 엔진에 알려,
	// FlowAbortMode 설정(EBTFlowAbortMode::LowerPriority)에 맞추어 자동 Abort/재실행하도록 함
	this->ConditionalFlowAbort(*BehaviorTreeComponent, EBTDecoratorAbortRequest::ConditionResultChanged);
	// -----------------

	// 일반적으로 콜백 이후에도 계속 관찰
	return EBlackboardNotificationResult::ContinueObserving;
}
