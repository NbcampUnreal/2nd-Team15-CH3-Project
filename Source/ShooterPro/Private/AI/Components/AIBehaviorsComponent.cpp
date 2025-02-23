#include "AI/Components/AIBehaviorsComponent.h"

#include "AI/AIGameplayTags.h"
#include "AI/EnemyAIBase.h"
#include "AI/EnemyAIController.h"
#include "AI/EnemyAILog.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"


UAIBehaviorsComponent::UAIBehaviorsComponent(): AttackTarget(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	AISensePriority.Add(EAISense::Damage, 1.0f);
	AISensePriority.Add(EAISense::Sight, 0.8);
	AISensePriority.Add(EAISense::Hearing, 0.4);
}

void UAIBehaviorsComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterRef = Cast<AEnemyAIBase>(GetOwner());
	if (CharacterRef)
	{
		AIControllerRef = Cast<AEnemyAIController>(UAIBlueprintHelperLibrary::GetAIController(CharacterRef));
	}
}

void UAIBehaviorsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UAIBehaviorsComponent::IsTriggerEnabled(ECombatTriggerFlags Trigger) const
{
	// Trigger가 이미 1, 2, 4, 8 등의 비트값을 갖고 있으므로 그대로 마스크 연산
	const uint8 MaskValue = static_cast<uint8>(Trigger);
	return (CombatTriggerMask & MaskValue) != 0;
}

bool UAIBehaviorsComponent::CanChangeState(FGameplayTag ChangeState)
{
	// FGameplayTag CurrentState = AIControllerRef->GetCurrentStateTag();

	if (ChangeState == AIGameplayTags::AIState_Combat)
	{
		if (CurrentState != AIGameplayTags::AIState_Dead)
		{
			return (IsValid(AttackTarget));
		}
	}
	else if (ChangeState == AIGameplayTags::AIState_Idle)
	{
		return true;
	}
	else if (ChangeState == AIGameplayTags::AIState_Seeking)
	{
		if (PreviousState == AIGameplayTags::AIState_Combat)
		{
			return true;
		}
		if (PreviousState == AIGameplayTags::AIState_Idle)
		{
			return false;
		}
	}
	else if (ChangeState == AIGameplayTags::AIState_Dead)
	{
	}
	else if (ChangeState == AIGameplayTags::AIState_Seeking)
	{
	}
	else if (ChangeState == AIGameplayTags::AIState_Disabled)
	{
	}


	return false;
}

void UAIBehaviorsComponent::UpdateState(FGameplayTag UpdateState)
{
	PreviousState = CurrentState;
	CurrentState = UpdateState;
	AIControllerRef->UpdateBlackboard_State(UpdateState);

	if (UpdateState == AIGameplayTags::AIState_Combat)
	{
		//위 과정과 동일
	}
	else if (UpdateState == AIGameplayTags::AIState_Seeking)
	{
		SetStateAsSeeking();

		//ToDo::비헤이비어 트리에서 할 것입니다.
		// GetWorld()->GetTimerManager().SetTimer(SeekTimerHandle, this, &UAIBehaviorsComponent::SetStateAsSeeking, TimeToSeekAfterLosingSight, false);
	}
	if (UpdateState == AIGameplayTags::AIState_Idle)
	{
		//위 과정과 동일
	}
}

void UAIBehaviorsComponent::HandleForgotActor(const FPerceivedActorInfo& PerceivedActorInfo)
{
	// KnownSeenActors.Remove(Actor);
	//
	// if (Actor == AttackTarget)
	// {
	// 	if (GetCurrentState() != EAIState::Seeking)
	// 		BlackBoardUpdate_State(EAIState::Idle);
	// }
}

void UAIBehaviorsComponent::HandleSensedSight(const FPerceivedActorInfo& PerceivedActorInfo)
{
	// AI_ENEMY_SCREEN_LOG_LOG(5.0f, "시야로 감지했습니다");

	// 이미 공격 대상이 설정되어 있다면, 추가적인 타겟 설정을 하지 않음
	if (AttackTarget == PerceivedActorInfo.DetectedActor)
		return;

	// 같은 팀의 액터일 경우 아무런 처리를 하지 않음
	if (AIControllerRef->OnSameTeam(PerceivedActorInfo.DetectedActor))
		return;

	// UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, SeekTimerHandle);

	ECombatTriggerFlags SightCombatTrigger = ECombatTriggerFlags::Sight;
	if (!IsTriggerEnabled(SightCombatTrigger))
		return;

	// 공격 가능한 타겟 목록에 추가
	if (!AttackableTargets.Contains(PerceivedActorInfo.DetectedActor))
	{
		AttackableTargets.AddUnique(PerceivedActorInfo.DetectedActor);
	}
}

void UAIBehaviorsComponent::HandleLostSight(const FPerceivedActorInfo& PerceivedActorInfo)
{
	// AI_ENEMY_SCREEN_LOG_LOG(5.0f, "시야로 감지한 것을 잃었습니다");

	if (AttackTarget == PerceivedActorInfo.DetectedActor)
	{
		if (!AttackableTargets.IsEmpty())
		{
			AttackableTargets.Remove(PerceivedActorInfo.DetectedActor);
		}
	}
}

void UAIBehaviorsComponent::HandleSensedSound(const FPerceivedActorInfo& PerceivedActorInfo)
{
	// EAIState CurrentState = GetCurrentState();
	// if (CurrentState == EAIState::Idle || CurrentState == EAIState::Investigating || CurrentState == EAIState::Seeking)
	// {
	// 	SetStateAsInvestigating(Location);
	// }
}

void UAIBehaviorsComponent::HandleSensedDamage(const FPerceivedActorInfo& PerceivedActorInfo)
{
	// if (OnSameTeam(Actor))
	// 	return;
	//
	// EAIState CurrentState = GetCurrentState();
	//
	// if (CurrentState == EAIState::Idle || CurrentState == EAIState::Investigating || CurrentState == EAIState::Seeking)
	// {
	// 	SetStateAsAttacking(Actor, false);
	// }
}

void UAIBehaviorsComponent::HandleLostSound(const FPerceivedActorInfo& PerceivedActorInfo)
{
}

void UAIBehaviorsComponent::HandleLostDamage(const FPerceivedActorInfo& PerceivedActorInfo)
{
}

void UAIBehaviorsComponent::SetStateAsSeeking()
{
	PreviousState = CurrentState;
	CurrentState = AIGameplayTags::AIState_Seeking;
	AIControllerRef->UpdateBlackboard_PointOfInterest(LastSenseHandle.LastKnownLocation);
	AIControllerRef->UpdateBlackboard_State(AIGameplayTags::AIState_Seeking);
	UKismetSystemLibrary::DrawDebugSphere(this, LastSenseHandle.LastKnownLocation, 150.0f, 12, FLinearColor::Red, 15.0f, 10.0f);
	// UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, SeekTimerHandle);
}

void UAIBehaviorsComponent::SetStateAsAttacking_Implementation(AActor* NewAttackTarget, bool bUseLastKnownAttackTarget)
{
	// 현재 AttackTarget이 유효하고 이전 대상을 사용하도록 지정된 경우 기존 AttackTarget 사용, 그렇지 않으면 새 대상 사용
	AActor* BestAttackTarget = (IsValid(AttackTarget) && bUseLastKnownAttackTarget) ? AttackTarget : NewAttackTarget;

	// // 유효한 공격 대상이 아닐 경우 상태를 Passive으로 전환하고 함수 종료
	// if (!IsValid(BestAttackTarget))
	// {
	// 	AIControllerRef->BlackBoardUpdate_State(EAIState::Idle);
	// 	return;
	// }

	// // 공격 대상이 데미지 인터페이스를 구현했는지 확인하고, 이미 사망한 대상인 경우 상태를 Passive으로 전환
	// if (BestAttackTarget->Implements<UInterface_Damageable>())
	// {
	// 	if (IInterface_Damageable::Execute_IsDead(BestAttackTarget))
	// 	{
	// 		AIControllerRef->BlackBoardUpdate_State(EAIState::Idle);
	// 		return;
	// 	}
	// }

	// Blackboard에 공격 대상 및 상태 업데이트
	// AIControllerRef->BlackBoardUpdate_AttackTarget(BestAttackTarget);
	// AIControllerRef->BlackBoardUpdate_State(EAIState::Combat);
	// AttackTarget = BestAttackTarget;
}

void UAIBehaviorsComponent::HandlePerceptionUpdated(const FPerceivedActorInfo& PerceivedActorInfo)
{
	LastSenseHandle = PerceivedActorInfo;

	if (PerceivedActorInfo.DetectedSense == EAISense::Sight)
	{
		if (PerceivedActorInfo.bCurrentlySensed)
			HandleSensedSight(PerceivedActorInfo);
		else
			HandleLostSight(PerceivedActorInfo);
	}

	else if (PerceivedActorInfo.DetectedSense == EAISense::Hearing)
	{
		if (PerceivedActorInfo.bCurrentlySensed)
			HandleSensedSound(PerceivedActorInfo);
		else
			HandleSensedSound(PerceivedActorInfo);
	}

	else if (PerceivedActorInfo.DetectedSense == EAISense::Damage)
	{
		if (PerceivedActorInfo.bCurrentlySensed)
			HandleSensedDamage(PerceivedActorInfo);
		else
			HandleLostDamage(PerceivedActorInfo);
	}
}

void UAIBehaviorsComponent::HandlePerceptionForgotten(const FPerceivedActorInfo& PerceivedActorInfo)
{
}
