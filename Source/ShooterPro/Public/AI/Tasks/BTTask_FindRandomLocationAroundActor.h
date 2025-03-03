#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindRandomLocationAroundActor.generated.h"

/**
 * 블랙보드에서 Actor와 float 키를 받아,
 * 그 Actor 주변 float 거리 내의 임의의 이동 가능한 위치를 찾아 블랙보드에 저장하는 BTTask입니다.
 * 위치를 찾으면 Succeeded, 그렇지 않으면 Failed를 반환합니다.
 */
UCLASS()
class SHOOTERPRO_API UBTTask_FindRandomLocationAroundActor : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindRandomLocationAroundActor();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	//스폰 된 위치 주변으로의 랜덤을 원하는가?
	UPROPERTY(EditAnywhere, Category="RandomLocationAround")
	bool bStartOrigin = false;

	// 블랙보드에서 가져올 Actor 키
	UPROPERTY(EditAnywhere, Category="RandomLocationAround", meta=(EditCondition="bStartOrigin"))
	FBlackboardKeySelector TargetActorKey;

	// 블랙보드에서 가져올 이동 거리(반경) 키
	UPROPERTY(EditAnywhere, Category="RandomLocationAround")
	FBlackboardKeySelector DistanceRangeRadiusKey;

	// 찾은 임의의 위치를 저장할 블랙보드 키
	UPROPERTY(EditAnywhere, Category="RandomLocationAround")
	FBlackboardKeySelector RandomLocationKey;
};
