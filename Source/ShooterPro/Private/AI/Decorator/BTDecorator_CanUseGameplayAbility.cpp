// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Decorator/BTDecorator_CanUseGameplayAbility.h"

#include "AIController.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTDecorator_CanUseGameplayAbility::UBTDecorator_CanUseGameplayAbility()
{
	NodeName = TEXT("Can Use Gameplay Ability?");
	// 블랙보드 Key 사용 시 아래 코드를 통해 "Notify" 하도록 설정
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;

	// 기본적으로 Abort를 끄거나 켤 수 있음 (필요 시)
	// FlowContorl이 필요하면 bNotifyTick, bNotifyDeactivation 등을 세팅
}

bool UBTDecorator_CanUseGameplayAbility::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	// 1) AIController 및 Controlled Pawn 가져오기
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return false;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn)
		return false;

	// 2) 블랙보드에서 타겟 액터 가져오기
	UObject* BBValue = OwnerComp.GetBlackboardComponent()->GetValueAsObject(TargetActorKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(BBValue);
	if (!TargetActor)
		return false;

	// 3) ASC(AbilitySystemComponent) 가져오기
	UGSCAbilitySystemComponent* ASC = TargetActor->FindComponentByClass<UGSCAbilitySystemComponent>();
	if (!ASC)
		return false;

	// 4) AbilityTag로부터 사용 가능한 AbilitySpec 조회
	TArray<FGameplayAbilitySpec*> FoundSpecs;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(AbilityTag), FoundSpecs);

	bool bCanUseAbility = false;
	for (FGameplayAbilitySpec* Spec : FoundSpecs)
	{
		if (Spec && Spec->Ability)
		{
			const FGameplayAbilityActorInfo* ActorInfoPtr = ASC->AbilityActorInfo.Get();
			if (!ActorInfoPtr)
			{
				return false; // 액터 정보가 유효하지 않으면 바로 false
			}

			// 실제 Ability의 CanActivateAbility 호출
			if (Spec->Ability->CanActivateAbility(Spec->Handle, ActorInfoPtr, nullptr, nullptr, nullptr))
			{
				bCanUseAbility = true;
				break;
			}
		}
	}

	return bCanUseAbility;
}

#if WITH_EDITOR
FName UBTDecorator_CanUseGameplayAbility::GetNodeIconName() const
{
	// 에디터에서 아이콘 보일 때
	return FName("BTEditor.Graph.BTNode.Decorator.ForceSuccess.Icon");
}
#endif
