
#include "AI/Decorator/BTDecorator_CanActivateOneOfAbilities.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "GameplayAbilitySpec.h"
#include "Abilities/GSCAbilitySystemComponent.h"

class UAbilitySystemComponent;
class IAbilitySystemInterface;

UBTDecorator_CanActivateOneOfAbilities::UBTDecorator_CanActivateOneOfAbilities()
{
	NodeName = TEXT("Can Activate One Of Abilities?");
	// 필요시 데코레이터가 블랙보드 값 변경에 반응하도록 설정
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

bool UBTDecorator_CanActivateOneOfAbilities::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	// 부모 호출
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

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

	// AbilitySystem 가져오기
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(Pawn);
	if (!AbilitySystemInterface)
	{
		return false;
	}

	UGSCAbilitySystemComponent* ASC = Cast<UGSCAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
	if (!ASC)
	{
		return false;
	}

	// 1) AbilityTags 중 하나라도 매칭되는 AbilitySpec을 찾는다
	//    GetActivatableGameplayAbilitySpecsByAnyMatchingTags로 여러 태그 중 하나라도 일치하면 가져옴
	TArray<FGameplayAbilitySpec*> MatchingSpecs;
	ASC->GetActivatableGameplayAbilitySpecsByAnyMatchingTags(AbilityTags, MatchingSpecs, /*bOnlyAbilitiesThatSatisfyTagRequirements=*/false);

	// 2) 찾은 AbilitySpec 중 실제로 CanActivateAbility가 가능한 것이 하나라도 있으면 true
	for (FGameplayAbilitySpec* Spec : MatchingSpecs)
	{
		if (Spec && Spec->Ability)
		{
			const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
			if (ActorInfo && Spec->Ability->CanActivateAbility(Spec->Handle, ActorInfo, nullptr, nullptr, nullptr))
			{
				return true; // 조건 만족(하나라도 활성 가능)
			}
		}
	}

	// 전부 불가능하면 false
	return false;
}

#if WITH_EDITOR
FName UBTDecorator_CanActivateOneOfAbilities::GetNodeIconName() const
{
	// 에디터 아이콘은 예시로 ForceSuccess 아이콘을 사용
	return FName("BTEditor.Graph.BTNode.Decorator.ForceSuccess.Icon");
}
#endif