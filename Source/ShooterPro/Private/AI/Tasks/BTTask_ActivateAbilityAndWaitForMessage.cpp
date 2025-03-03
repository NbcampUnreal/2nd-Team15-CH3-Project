#include "AI/Tasks/BTTask_ActivateAbilityAndWaitForMessage.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"
#include "Abilities/GameplayAbility.h"
#include "AI/EnemyAILog.h"
#include "AI/EnemyAITypes.h"
#include "GameFramework/AsyncAction_ListenForGameplayMessage.h"


UBTTask_ActivateAbilityAndWaitForMessage::UBTTask_ActivateAbilityAndWaitForMessage()
{
	NodeName = TEXT("ActivateAbilityAndWaitForMessage");
	bCreateNodeInstance = true;

	// Task가 종료될 때 OnTaskFinished가 호출되도록 설정
	// (bNotifyTaskFinished = true와 동일, 매크로 방식)
	INIT_TASK_NODE_NOTIFY_FLAGS();
}

EBTNodeResult::Type UBTTask_ActivateAbilityAndWaitForMessage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	MyOwnerComp = &OwnerComp;

	// 1) Pawn(혹은 Controller)에서 ASC 찾기
	APawn* Pawn = OwnerComp.GetAIOwner() ? OwnerComp.GetAIOwner()->GetPawn() : nullptr;
	if (!Pawn)
	{
		AI_ENEMY_LOG_WARNING("%s - No Pawn for AIOwner", *GetName());
		return EBTNodeResult::Failed;
	}

	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(Pawn);
	if (!AbilitySystemInterface)
	{
		AI_ENEMY_LOG_WARNING("%s - Pawn doesn't implement AbilitySystemInterface", *GetName());
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = AbilitySystemInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		AI_ENEMY_LOG_WARNING("%s - AbilitySystemComponent is null", *GetName());
		return EBTNodeResult::Failed;
	}

	// 2) bUseAbilityTag에 따라: AbilityTag vs AbilityClass
	bool bActivationSucceeded;
	FGameplayAbilitySpecHandle FoundSpecHandle;
	FGameplayTagContainer AbilityTags;

	if (bUseAbilityTag)
	{
		// Tag 기반 Activate
		// AI_ENEMY_LOG_DISPLAY("%s - Activating abilities by tag: %s", *GetName(), *AbilityTag.ToString());

		bActivationSucceeded = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag), /*bAllowRemoteActivation=*/false);
		if (!bActivationSucceeded)
		{
			AI_ENEMY_LOG_WARNING("%s - TryActivateAbilitiesByTag(%s) Failed or not allowed", *GetName(), *AbilityTag.ToString());
			return EBTNodeResult::Failed;
		}

		// 실제로 활성화된 AbilitySpec 찾기
		TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;
		ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(FGameplayTagContainer(AbilityTag), MatchingGameplayAbilities, /*bOnlyAbilitiesThatSatisfyTagRequirements=*/false);

		if (MatchingGameplayAbilities.Num() == 0)
		{
			AI_ENEMY_LOG_WARNING("%s - No AbilitySpec found for Tag: %s", *GetName(), *AbilityTag.ToString());
			return EBTNodeResult::Failed;
		}

		// 첫 번째 Spec을 대표로 사용
		FGameplayAbilitySpec* FirstSpec = MatchingGameplayAbilities[0];
		FoundSpecHandle = FirstSpec->Handle;
		AbilityTags = FirstSpec->Ability->GetAssetTags();
	}
	else
	{
		// AbilityClass 기반 Activate
		if (!AbilityToActivate)
		{
			AI_ENEMY_LOG_WARNING("%s - AbilityToActivate is invalid.", *GetName());
			return EBTNodeResult::Failed;
		}

		AI_ENEMY_LOG_DISPLAY("%s - Activating ability by class: %s", *GetName(), *AbilityToActivate->GetName());

		bActivationSucceeded = ASC->TryActivateAbilityByClass(AbilityToActivate, /*bAllowRemoteActivation=*/false);
		if (!bActivationSucceeded)
		{
			AI_ENEMY_LOG_WARNING("%s - Ability Activation Failed or not allowed", *GetName());
			return EBTNodeResult::Failed;
		}

		// Activated AbilitySpecHandle 찾기 + 해당 어빌리티 태그 얻기
		const TArray<FGameplayAbilitySpec>& AllAbilities = ASC->GetActivatableAbilities();
		for (const FGameplayAbilitySpec& Spec : AllAbilities)
		{
			if (Spec.Ability && Spec.Ability->GetClass() == AbilityToActivate)
			{
				FoundSpecHandle = Spec.Handle;
				AbilityTags = Spec.Ability->GetAssetTags();
				break;
			}
		}

		if (!FoundSpecHandle.IsValid())
		{
			AI_ENEMY_LOG_WARNING("%s - Could not find the SpecHandle for the activated Ability.", *GetName());
			return EBTNodeResult::Failed;
		}
	}

	WaitingAbilitySpecHandle = FoundSpecHandle;

	// 메시지 대기용 태그 추출
	ListeningTag = ExtractFirstTagOrDefault(AbilityTags);
	// AI_ENEMY_LOG_DISPLAY("%s - ListeningTag: %s", *GetName(), *ListeningTag.ToString());

	// 3) 메시지 수신 대기(AsyncListener)
	{
		// 우리가 실제로 수신할 구조체: FEnemyAbilityEndedPayload
		UScriptStruct* PayloadType = FEnemyAbilityEndedPayload::StaticStruct();
		if (!PayloadType)
		{
			AI_ENEMY_LOG_ERROR("PayloadType is invalid in %s", *GetName());
			return EBTNodeResult::Failed;
		}
		UWorld* MyWorld = OwnerComp.GetWorld();
		AsyncListener = UAsyncAction_ListenForGameplayMessage::ListenForGameplayMessages(
			MyWorld, // WorldContextObject
			ListeningTag, // 대기할 태그
			PayloadType,
			EGameplayMessageMatch::ExactMatch
		);

		if (!AsyncListener)
		{
			// AI_ENEMY_LOG_WARNING("%s - Failed to create UAsyncAction_ListenForGameplayMessage", *GetName());
			return EBTNodeResult::Failed;
		}

		AsyncListener->OnMessageReceived.AddDynamic(this, &UBTTask_ActivateAbilityAndWaitForMessage::OnMessageReceived);
		AsyncListener->Activate();
	}

	// 아직 메시지가 오지 않았으므로, InProgress 상태로 대기
	return EBTNodeResult::InProgress;
}

void UBTTask_ActivateAbilityAndWaitForMessage::OnMessageReceived(UAsyncAction_ListenForGameplayMessage* ProxyObject, FGameplayTag ActualChannel)
{
	if (!MyOwnerComp)
	{
		return;
	}

	// AI_ENEMY_LOG_LOG("OnMessageReceived - Received Tag=[%s], ListeningTag=[%s]", *ActualChannel.ToString(), *ListeningTag.ToString());

	// 메시지 구조체 타입이 우리가 원하는 FEnemyAbilityEndedPayload인지 확인
	const UScriptStruct* DesiredStruct = FEnemyAbilityEndedPayload::StaticStruct();
	if (ProxyObject && ProxyObject->GetMessageStructType() == DesiredStruct)
	{
		if (const void* DataPtr = ProxyObject->GetReceivedMessagePayloadPtr())
		{
			if (const FEnemyAbilityEndedPayload* PayloadPtr = reinterpret_cast<const FEnemyAbilityEndedPayload*>(DataPtr))
			{
				// AI_ENEMY_SCREEN_LOG_LOG(5.0f,
				//                         "Received FEnemyAbilityEndedPayload => AbilityName=%s, Tag=%s, Time=%.2f",
				//                         *PayloadPtr->EndedAbilityName,
				//                         *PayloadPtr->EndedAbilityTag.ToString(),
				//                         PayloadPtr->EndedTime
				// );

				// 예: OwnerActor 체크
				if (AActor* OwningActor = PayloadPtr->AbilityOwner.Get())
				{
					// AI_ENEMY_LOG_DISPLAY("AbilityOwner=[%s]", *OwningActor->GetName());
				}
			}
		}
	}

	// 태그가 ListeningTag와 같으면 Succeeded 처리
	if (ActualChannel == ListeningTag)
	{
		// AI_ENEMY_SCREEN_LOG_LOG(3.0f, "Message Tag matched => Task Succeeded!");
		FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_ActivateAbilityAndWaitForMessage::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

	// Listener 해제
	if (AsyncListener)
	{
		AsyncListener->Cancel();
		AsyncListener = nullptr;
	}
}

FGameplayTag UBTTask_ActivateAbilityAndWaitForMessage::ExtractFirstTagOrDefault(const FGameplayTagContainer& AbilityTags) const
{
	if (!AbilityTags.IsEmpty())
	{
		return *AbilityTags.CreateConstIterator();
	}
	return FGameplayTag::RequestGameplayTag(FName("MyGame.DefaultTag"));
}
