#include "AI/Service/BTService_SetAIStateOnActiveDeactive.h"

#include "AI/EnemyAIController.h"
#include "AI/Components/AIBehaviorsComponent.h"
#include "AI/Utility/EnemyAIBluePrintFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTService_SetAIStateOnActiveDeactive::UBTService_SetAIStateOnActiveDeactive()
{
	bSetOnActivation = true;
	// StateOnActivation = EAIState::Idle;

	bSetOnDeactivation = false;
	// StateOnDeactivation = EAIState::Idle;
	
	bNotifyTick= false;
	bNotifyBecomeRelevant = true; // OnBecomeRelevant 사용
	bNotifyCeaseRelevant = true; // OnCeaseRelevant 사용
}

void UBTService_SetAIStateOnActiveDeactive::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	
	if (bSetOnActivation)
	{
		if (AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner()))
		{
			// if (AIController->AIBehaviorComponent->CanChangeState(StateOnActivation)) ex:)Combat
				// AIController->UpdateBlackboard_State(StateOnActivation);
		}
	}
}

void UBTService_SetAIStateOnActiveDeactive::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	if (bSetOnDeactivation)
	{
		if (AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner()))
		{
			// if (AIController->AIBehaviorComponent->CanChangeState(StateOnDeactivation))
				// AIController->UpdateBlackboard_State(StateOnDeactivation);
		}
	}
}



