#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ActivateAbilityAndWaitForMessage.generated.h"

class UAsyncAction_ListenForGameplayMessage;
class UGameplayAbility;

/**
 * 1) 주어진 AbilityClass를 활성화 시도
 * 2) 어빌리티가 활성화되면, 해당 어빌리티의 태그를 ListenForGameplayMessage
 * 3) EnemyAIBase(등)에서 어빌리티 종료 시점에 브로드캐스트되는 메시지를 수신
 * 4) 메시지 태그가 활성화한 어빌리티 태그와 같다면 Task를 Succeed
 */
UCLASS()
class SHOOTERPRO_API UBTTask_ActivateAbilityAndWaitForMessage : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ActivateAbilityAndWaitForMessage();

protected:
	//~ Begin BTTaskNode interface
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	//~ End BTTaskNode interface

private:
	/** 메시지 도착 시 실행될 함수 (OnMessageReceived) */
	UFUNCTION()
	void OnMessageReceived(UAsyncAction_ListenForGameplayMessage* ProxyObject, FGameplayTag ActualChannel);

	/** 
	 * @brief 주어진 어빌리티에서 첫 번째 태그를 가져온다. 
	 * 비어있으면 FGameplayTag::RequestGameplayTag("MyGame.DefaultTag") 반환
	 */
	FGameplayTag ExtractFirstTagOrDefault(const FGameplayTagContainer& AbilityTags) const;

public:
	/** 
	 * true 이면 AbilityTag를 사용하여 어빌리티 실행(ASC->TryActivateAbilitiesByTag)
	 * false 이면 AbilityClass로 실행(ASC->TryActivateAbilityByClass)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability")
	bool bUseAbilityTag = false;

	/** 어빌리티 태그로 실행할 경우: 이 태그가 적용된 AbilitySpec들을 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability", meta=(EditCondition="bUseAbilityTag", EditConditionHides))
	FGameplayTag AbilityTag;

	/** 어빌리티 클래스로 실행할 경우: 이 클래스를 활성화 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability", meta=(EditCondition="!bUseAbilityTag", EditConditionHides))
	TSubclassOf<UGameplayAbility> AbilityToActivate;

private:
	/** 메시지 수신 대기용 Async Node 인스턴스 */
	UPROPERTY()
	UAsyncAction_ListenForGameplayMessage* AsyncListener = nullptr;

	/** 우리가 활성화한 어빌리티의 핸들 */
	FGameplayAbilitySpecHandle WaitingAbilitySpecHandle;

	/** 우리가 활성화한 어빌리티의 태그 (첫 번째 태그) */
	FGameplayTag ListeningTag;

	/** BehaviorTree에서 LatentFinish를 위해 저장해 둠 */
	UPROPERTY()
	UBehaviorTreeComponent* MyOwnerComp = nullptr;
};
