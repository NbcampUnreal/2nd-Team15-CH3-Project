#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SelectAttackTarget.generated.h"

class AEnemyAIController;
class UAIBehaviorsComponent;

/**
 * @brief 공격 가능한 타겟을 선택하는 서비스.
 * AttackableTargets에서 가장 적합한 타겟을 선정하여 AttackTarget으로 설정.
 */
UCLASS()
class SHOOTERPRO_API UBTService_SelectAttackTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SelectAttackTarget();

protected:
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY()
	TWeakObjectPtr<AEnemyAIController> CachedAIControllers;

	UPROPERTY()
	TWeakObjectPtr<UAIBehaviorsComponent> CachedAIBehaviorsComps;
};
