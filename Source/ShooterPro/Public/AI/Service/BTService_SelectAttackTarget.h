

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
	/** 기본 생성자 */
	UBTService_SelectAttackTarget();

protected:
	/** 이 서비스가 해당 BehaviorTree에 연결될 때 호출됩니다. */
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 매 Tick마다 호출됩니다. AttackableTargets에서 가장 적합한 타겟을 선택하여 AttackTarget을 설정합니다. */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/** 이 서비스가 더 이상 relevant하지 않게 될 때 호출됩니다. */
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	/** 공격 가능한 타겟을 선택하는 함수 */
	void SelectAttackTarget(UBehaviorTreeComponent& OwnerComp);

private:
	TWeakObjectPtr<AEnemyAIController> CachedAIController;  
	TWeakObjectPtr<UAIBehaviorsComponent> CachedAIBehaviorsComp;  

};
