// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTTask_MoveRandomAroundActor.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "AI/EnemyAILog.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveRandomAroundActor::UBTTask_MoveRandomAroundActor(): CachedOwnerComp(nullptr)
{
	NodeName = TEXT("Move Random Around Range Actor");
	// 이 Task가 이동 완료 이벤트를 받을 것이므로 bNotifyBecomeRelevant, bNotifyTick 등은 따로 설정 안 해도 됨
}

EBTNodeResult::Type UBTTask_MoveRandomAroundActor::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BBComp = AIController->GetBlackboardComponent();
	if (!BBComp)
	{
		return EBTNodeResult::Failed;
	}

	CachedOwnerComp = &OwnerComp;

	// 블랙보드에서 Actor 값, float 값 가져오기
	AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		AI_ENEMY_SCREEN_LOG_ERROR(5.0f, "UBTTASK_MOVERANDOMAROUNDACTOR : 유효한 TargetActor가 없습니다. FALID");
		return EBTNodeResult::Failed;
	}

	float Radius = BBComp->GetValueAsFloat(DistanceRangeRadiusKey.SelectedKeyName);
	if (Radius <= 0.f)
	{
		AI_ENEMY_SCREEN_LOG_ERROR(5.0f, "UBTTASK_MOVERANDOMAROUNDACTOR : RADIUS는 0 또는 음수입니다.");
		return EBTNodeResult::Failed;
	}

	// 네비게이션 시스템 가져오기
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(TargetActor);
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}

	// TargetActor 위치 주변에서 랜덤으로 갈 수 있는 지점 찾기
	FVector Origin = TargetActor->GetActorLocation();
	FNavLocation RandomLocation;

	bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, Radius, RandomLocation);
	if (!bFound)
	{
		AI_ENEMY_SCREEN_LOG_ERROR(5.0f, "UBTTASK_MOVERANDOMAROUNDACTOR : 임의의 도달 가능 지점을 찾지 못했습니다.");
		return EBTNodeResult::Failed;
	}

	// AI 이동 (기본 허용 오차는 50.0f 정도로)
	float AcceptanceRadius = 50.0f;
	EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(RandomLocation.Location, AcceptanceRadius);

	if (MoveResult == EPathFollowingRequestResult::RequestSuccessful)
	{
		// 이동 완료를 델리게이트로 받기 위해 바인딩
		// TaskNode와 Controller 사이에 연결 (OwnerComp 보존을 위해 람다 or UFunction 형태)
		AIController->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_MoveRandomAroundActor::OnMoveCompleted);


		// 델리게이트 핸들 보관 (필요시 사용)
		// AIController->ReceiveMoveCompleted.AddDynamic(this, &UBTTask_MoveRandomAroundActor::OnMoveCompleted, &OwnerComp);

		// 이동이 진행 중이므로 여기서는 InProgress
		return EBTNodeResult::InProgress;
	}
	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_MoveRandomAroundActor::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 이동 중단
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
		// 혹시나 델리게이트가 바인딩되어 있다면 해제
		AIController->ReceiveMoveCompleted.RemoveDynamic(this, &UBTTask_MoveRandomAroundActor::OnMoveCompleted);
	}

	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_MoveRandomAroundActor::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	EBTNodeResult::Type NodeResult = (Result == EPathFollowingResult::Success) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;

	// FinishLatentTask를 호출하려면 UBehaviorTreeComponent*가 필요
	// ExecuteTask() 매개변수 OwnerComp를 멤버 변수로 캐싱해서 여기서 사용하거나
	// AIController->BrainComponent 등으로부터 가져올 수도 있음
	if (CachedOwnerComp)
	{
		FinishLatentTask(*CachedOwnerComp, NodeResult);
	}
}
