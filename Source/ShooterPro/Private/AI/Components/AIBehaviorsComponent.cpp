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
	//원하는 상태가 컴뱃이라면
	if (ChangeState == AIGameplayTags::AIState_Combat)
	{
		//내 현재상태가 죽음이 아니고 && 공격가능한 대상들이 하나라도 있다면
		if (CurrentState != AIGameplayTags::AIState_Dead && !AttackableTargets.IsEmpty())
		{
			return true;
		}
		
		if (CurrentState == AIGameplayTags::AIState_Dead)
		{
			return false;
		}
		
		return true;
	}


	if (ChangeState == AIGameplayTags::AIState_Idle)
	{
		if (ChangeState == AIGameplayTags::AIState_Combat)
		{
			return true;
		}
		
		return true;
	}

	if (ChangeState == AIGameplayTags::AIState_Seeking)
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

	if (ChangeState == AIGameplayTags::AIState_Dead)
	{
	}

	if (ChangeState == AIGameplayTags::AIState_Seeking)
	{
	}

	if (ChangeState == AIGameplayTags::AIState_Disabled)
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
	}
	if (UpdateState == AIGameplayTags::AIState_Idle)
	{
		//위 과정과 동일
	}
}

bool UAIBehaviorsComponent::IsInCombat()
{
	return CurrentState == AIGameplayTags::AIState_Combat;
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
	// 이미 공격 대상이 설정되어 있다면, 추가적인 타겟 설정을 하지 않음
	if (AttackTarget == PerceivedActorInfo.DetectedActor)
		return;

	// 같은 팀의 액터일 경우 아무런 처리를 하지 않음
	if (AIControllerRef->OnSameTeam(PerceivedActorInfo.DetectedActor))
		return;

	ECombatTriggerFlags SightCombatTrigger = ECombatTriggerFlags::Sight;
	if (!IsTriggerEnabled(SightCombatTrigger))
		return;

	// 공격 가능한 타겟 목록에 추가
	AttackableTargets.AddUnique(PerceivedActorInfo.DetectedActor);
}

void UAIBehaviorsComponent::HandleLostSight(const FPerceivedActorInfo& PerceivedActorInfo)
{
	AI_ENEMY_SCREEN_LOG_LOG(10.0f,"시야에서 놓쳤습니다. 놓친적이 없어서 이 로그가 호출되면 안됩니다");
	if (AttackableTargets.Contains(PerceivedActorInfo.DetectedActor))
		AttackableTargets.Remove(PerceivedActorInfo.DetectedActor);
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

float UAIBehaviorsComponent::GetRealRotationRate()
{
	return CurrentState == AIGameplayTags::AIState_Combat ? CombatRotationRate : InitialRotationRate;
}

void UAIBehaviorsComponent::SetStateAsAttacking()
{
	// 공격 가능한 타겟 목록을 가져옵니다.

	// 타겟이 없다면, 리턴
	if (AttackableTargets.IsEmpty())
	{
		AttackTarget = nullptr;
		AIControllerRef->UpdateBlackboard_AttackTarget(AttackTarget);
		return;
	}

	if (AttackableTargets.Num() == 1)
	{
		// 최적의 공격 대상을 설정
		AttackTarget = AttackableTargets[0];
		AIControllerRef->UpdateBlackboard_AttackTarget(AttackTarget);
		return;
	}

	//1마리가 만약 아니라면

	// 가장 가까운 타겟을 선택
	AActor* BestTarget = nullptr;
	float ShortestDistance = FLT_MAX;

	// 기본적으로는 거리 기반으로 가장 가까운 타겟을 선택
	for (AActor* PotentialTarget : AttackableTargets)
	{
		if (!IsValid(PotentialTarget))
			continue;

		//ToDo::나중에 죽음에 대한 것도 처리해줘야합니다.

		float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialTarget->GetActorLocation());

		// 가장 가까운 타겟을 선택
		if (Distance < ShortestDistance)
		{
			ShortestDistance = Distance;
			BestTarget = PotentialTarget;
		}
	}

	// 최적의 공격 대상을 설정
	AttackTarget = BestTarget;
	AIControllerRef->UpdateBlackboard_AttackTarget(BestTarget);
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
