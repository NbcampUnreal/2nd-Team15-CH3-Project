#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_HandlePerception.generated.h"

// 전방 선언
class UProAIBehaviorsComponent;
class AEnemyAIController;

struct FPerceivedActorInfo;

/**
 * @brief
 * 이 서비스는 AI가 주변을 탐지하고, 감지된 객체에 대해 필요한 행동을 취하기 위해 사용됩니다.
 * 예를 들어, AI가 적을 감지하거나 적의 위치를 잃었을 때 특정 행동을 취할 수 있도록 처리합니다.
 */
UCLASS()
class SHOOTERPRO_API UBTService_HandlePerception : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_HandlePerception();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	TWeakObjectPtr<AEnemyAIController> CachedController; // 캐시된 AIController
	TWeakObjectPtr<UProAIBehaviorsComponent> CachedBehaviorsComp; // 캐시된 AIBehaviorsComponent
};
