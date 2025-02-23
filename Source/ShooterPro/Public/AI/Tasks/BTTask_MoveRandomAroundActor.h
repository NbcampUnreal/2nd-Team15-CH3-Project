#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveRandomAroundActor.generated.h"

/**
 * 블랙보드에서 Actor와 float 키를 받아,
 * 그 Actor 주변 float 거리 내의 임의 지점으로 AI를 이동시키는 BTTask입니다.
 */
UCLASS()
class SHOOTERPRO_API UBTTask_MoveRandomAroundActor : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveRandomAroundActor();

protected:
	
	// 실제 이동 명령이 완료되었을 때 ReceiveMoveCompleted 델리게이트로 처리하려면 필요
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	// 이동 완료 시점에 호출되는 델리게이트 함수
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

protected:
	// 블랙보드에서 가져올 Actor 키
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// 블랙보드에서 가져올 이동 거리(반경) 키
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector DistanceRangeRadiusKey;

	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
	
};
