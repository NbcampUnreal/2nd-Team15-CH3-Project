#include "AI/Tasks/BTTask_FindRandomLocationAroundActor.h"


#include "AIController.h"
#include "NavigationSystem.h"
#include "AI/EnemyAIController.h"
#include "AI/Utility/EnemyAIBluePrintFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_FindRandomLocationAroundActor::UBTTask_FindRandomLocationAroundActor()
{
	NodeName = TEXT("Find Random Location Around Actor");
}

EBTNodeResult::Type UBTTask_FindRandomLocationAroundActor::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// AI 컨트롤러 가져오기
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	// 블랙보드 컴포넌트 가져오기
	UBlackboardComponent* BBComp = AIController->GetBlackboardComponent();
	if (!BBComp)
	{
		return EBTNodeResult::Failed;
	}

	// 블랙보드에서 TargetActor와 반경 값 가져오기
	AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		return EBTNodeResult::Failed;
	}

	float Radius = BBComp->GetValueAsFloat(DistanceRangeRadiusKey.SelectedKeyName);
	if (Radius <= 0.f)
	{
		return EBTNodeResult::Failed;
	}

	// 네비게이션 시스템 가져오기
	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(TargetActor);
	if (!NavSys)
	{
		return EBTNodeResult::Failed;
	}


	// TargetActor 주변에서 이동 가능한 임의의 위치 찾기
	FVector Origin = bStartOrigin ? BBComp->GetValueAsVector(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_StartLocation()) : TargetActor->GetActorLocation();
	FNavLocation RandomLocation;

	bool bFound = NavSys->GetRandomReachablePointInRadius(Origin, Radius, RandomLocation);
	if (!bFound)
	{
		return EBTNodeResult::Failed;
	}

	// 찾은 위치를 블랙보드에 저장
	BBComp->SetValueAsVector(RandomLocationKey.SelectedKeyName, RandomLocation.Location);

	return EBTNodeResult::Succeeded;
}
