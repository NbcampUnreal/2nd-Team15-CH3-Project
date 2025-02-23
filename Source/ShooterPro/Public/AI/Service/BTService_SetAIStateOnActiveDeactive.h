#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetAIStateOnActiveDeactive.generated.h"

/**
 * @class UBTService_SetAIStateOnActiveDeactive
 * 
 * 이 서비스는 Behavior Tree에서 활성화/비활성화될 때 특정 상태(EAIState)를 설정하는 역할을 합니다. 
 * - OnBecomeRelevant(활성화) 시, EAIState를 원하는 값으로 설정합니다.
 * - OnCeaseRelevant(비활성화) 시, EAIState를 원하는 값으로 설정합니다.
 * 
 * 이 서비스는 AI가 특정 조건에 따라 상태를 전환하는 데 유용하며, AI의 행동을 제어하는 중요한 요소로 활용됩니다.
 * 예를 들어, Combat 상태로 들어갈 때나 특정 행동이 종료되었을 때 상태를 바꾸는 방식으로 사용할 수 있습니다.
 */
UCLASS()
class SHOOTERPRO_API UBTService_SetAIStateOnActiveDeactive : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_SetAIStateOnActiveDeactive();

protected:
	/**
	 * 서비스가 활성화될 때 호출되는 함수.
	 * 이 함수는 Behavior Tree가 활성화되었을 때 설정된 상태(StateOnActivation)를 Blackboard에 적용합니다.
	 * 
	 * @param OwnerComp Behavior Tree 컴포넌트
	 * @param NodeMemory 노드 메모리 (현재 노드의 상태를 관리)
	 */
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/**
	 * 서비스가 비활성화될 때 호출되는 함수.
	 * 이 함수는 Behavior Tree가 비활성화되었을 때 설정된 상태(StateOnDeactivation)를 Blackboard에 적용합니다.
	 * 
	 * @param OwnerComp Behavior Tree 컴포넌트
	 * @param NodeMemory 노드 메모리 (현재 노드의 상태를 관리)
	 */
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** 서비스가 활성화될 때 상태를 변경할지 여부를 결정하는 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI State Service")
	bool bSetOnActivation;

	/** 활성화될 때 설정할 EAIState 값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI State Service", meta=(EditCondition="bSetOnActivation"))
	FGameplayTag StateOnActivation;

	/** 서비스가 비활성화될 때 상태를 변경할지 여부를 결정하는 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI State Service")
	bool bSetOnDeactivation;

	/** 비활성화될 때 설정할 EAIState 값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI State Service", meta=(EditCondition="bSetOnDeactivation"))
	FGameplayTag StateOnDeactivation;
};
