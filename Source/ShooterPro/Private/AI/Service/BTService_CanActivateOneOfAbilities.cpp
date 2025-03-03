// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Service/BTService_CanActivateOneOfAbilities.h"

#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_CanActivateOneOfAbilities::UBTService_CanActivateOneOfAbilities()
{
	NodeName = TEXT("Check Activatable Ability (Service)");
	// 이 값들로 Service 호출 간격, 무작위 편차 등을 블루프린트에서 지정 가능.
	Interval = 1.0f; // 1초마다 검사
	RandomDeviation = 0.2f;
}

void UBTService_CanActivateOneOfAbilities::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 1) AIController & Pawn
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return;
	}

	// 2) AbilitySystem 가져오기
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(Pawn);
	if (!AbilitySystemInterface)
	{
		return;
	}

	UGSCAbilitySystemComponent* ASC = Cast<UGSCAbilitySystemComponent>(AbilitySystemInterface->GetAbilitySystemComponent());
	if (!ASC)
	{
		return;
	}

	// 3) AbilityTags 중 하나라도 매칭되는 AbilitySpec 찾기
	TArray<FGameplayAbilitySpec*> MatchingSpecs;
	ASC->GetActivatableGameplayAbilitySpecsByAnyMatchingTags(AbilityTags, MatchingSpecs, /*bOnlyAbilitiesThatSatisfyTagRequirements=*/false);

	// 4) 찾은 AbilitySpec 중 실제 CanActivateAbility가 가능한지 검사
	bool bHasActivableAbility = false;
	for (FGameplayAbilitySpec* Spec : MatchingSpecs)
	{
		if (Spec && Spec->Ability)
		{
			const FGameplayAbilityActorInfo* ActorInfo = ASC->AbilityActorInfo.Get();
			if (ActorInfo && Spec->Ability->CanActivateAbility(Spec->Handle, ActorInfo, nullptr, nullptr, nullptr))
			{
				bHasActivableAbility = true;
				break; // 하나라도 가능하면 더 볼 필요 없음
			}
		}
	}

	// 5) 블랙보드에 결과 저장
	if (OwnerComp.GetBlackboardComponent())
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(bHasActivableAbilityKey.SelectedKeyName, bHasActivableAbility);
	}
}
