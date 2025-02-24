#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_HandlePerception.generated.h"

// 전방 선언
class UAIBehaviorsComponent;
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
	/** 기본 생성자 */
	UBTService_HandlePerception();

protected:
	/** 
	 * 이 서비스가 해당 BehaviorTree에 연결될 때 호출됩니다.
	 * AIController와 AIBehaviorsComponent를 캐싱하고, DetectionInfoManager의 델리게이트를 설정합니다.
	 */
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 
	 * 매 Tick마다 호출됩니다. 
	 * 감지 정보 업데이트를 위해 AIController의 UpdatePerception을 호출합니다.
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 
	 * 서비스가 더 이상 relevant하지 않게 될 때 호출됩니다.
	 * 델리게이트 등록을 해제하여 메모리 누수를 방지합니다.
	 */
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 
	 * PerceivedActorInfo가 갱신되었을 때 호출됩니다.
	 * AI의 감지된 정보를 AIBehaviorsComponent에 전달하여, 적절한 행동을 취하도록 유도합니다.
	 */
	UFUNCTION()
	void OnPerceptionUpdated(const FPerceivedActorInfo& PerceivedActorInfo);

	/** 
	 * 감지된 액터가 잊혀졌을 때 호출됩니다.
	 * 잊혀진 정보를 AIBehaviorsComponent에 전달하여, 해당 행동을 처리합니다.
	 */
	UFUNCTION()
	void OnTargetPerceptionForgotten(const FPerceivedActorInfo& PerceivedActorInfo);

private:
	/** 한 번만 찾고 캐싱해두기 위한 포인터들 */
	TWeakObjectPtr<AEnemyAIController> CachedController; // 캐시된 AIController
	TWeakObjectPtr<UAIBehaviorsComponent> CachedBehaviorsComp; // 캐시된 AIBehaviorsComponent
};

