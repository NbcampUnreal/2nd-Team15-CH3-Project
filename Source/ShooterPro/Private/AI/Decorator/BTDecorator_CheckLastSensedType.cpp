// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/BTDecorator_CheckLastSensedType.h"

#include "AIController.h"
#include "AI/Components/ProAIBehaviorsComponent.h"

UBTDecorator_CheckLastSensedType::UBTDecorator_CheckLastSensedType()
{
	NodeName = TEXT("Check Last Sensed Type");
	// 데코레이터가 블랙보드 값 변경을 감지해야 한다면 설정 가능:
	// bNotifyBecomeRelevant = true;
	// bNotifyCeaseRelevant = true;
}

bool UBTDecorator_CheckLastSensedType::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// 부모 함수 호출(디버그/기타)
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	// 1) AIController부터 Pawn(AEnemyAIBase) 얻기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return false;
	}

	// 2) AIBehaviorsComponent 찾기
	UProAIBehaviorsComponent* BehaviorsComp = Pawn->FindComponentByClass<UProAIBehaviorsComponent>();
	if (!BehaviorsComp)
	{
		return false;
	}

	// 3) 최근 감각을 가져와서, 우리가 원하는 감각(SenseToCheck)과 같은지 비교
	const FPerceivedActorInfo& LastInfo = BehaviorsComp->GetLastSenseHandle(); // 직접 Getter 함수를 만들어도 좋음
	// 만약 내부에서 bCurrentlySensed 등 추가 조건이 필요하면 함께 체크

	return (LastInfo.DetectedSense == SenseToCheck);
}

#if WITH_EDITOR
FName UBTDecorator_CheckLastSensedType::GetNodeIconName() const
{
	// 에디터에 보여줄 아이콘 (원하는 대로)
	return FName("BTEditor.Graph.BTNode.Decorator.Blackboard.Icon");
}
#endif