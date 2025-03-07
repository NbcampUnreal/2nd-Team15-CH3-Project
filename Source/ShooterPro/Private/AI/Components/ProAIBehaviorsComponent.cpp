#include "AI/Components/ProAIBehaviorsComponent.h"
#include "ProGmaeplayTag.h"
#include "AI/AIGameplayTags.h"
#include "AI/EnemyAIBase.h"
#include "AI/EnemyAIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"


UProAIBehaviorsComponent::UProAIBehaviorsComponent() : AttackTarget(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	AISensePriority.Add(EAISense::Damage, 1.0f);
	AISensePriority.Add(EAISense::Sight, 0.8f);
	AISensePriority.Add(EAISense::Hearing, 0.4f);
}

void UProAIBehaviorsComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterRef = Cast<AEnemyAIBase>(GetOwner());
	if (CharacterRef)
	{
		AIControllerRef = Cast<AEnemyAIController>(UAIBlueprintHelperLibrary::GetAIController(CharacterRef));
		if (AIControllerRef && AIControllerRef->GetDetectionInfoManager())
		{
			AIControllerRef->GetDetectionInfoManager()->OnAddPerceptionUpdated.AddDynamic(this, &UProAIBehaviorsComponent::HandlePerceptionUpdated);
			// AIControllerRef->GetDetectionInfoManager()->OnRemoveExpiredDetection.AddDynamic(this, &UProAIBehaviorsComponent::HandlePerceptionForgotten);
		}
	}
}

void UProAIBehaviorsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AIControllerRef && AIControllerRef->GetDetectionInfoManager())
	{
		AIControllerRef->GetDetectionInfoManager()->OnAddPerceptionUpdated.RemoveAll(this);
		AIControllerRef->GetDetectionInfoManager()->OnRemoveExpiredDetection.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}

void UProAIBehaviorsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UProAIBehaviorsComponent::InitializeBehavior(AEnemyAIController* PossedController)
{
	AIControllerRef = PossedController;
	AIControllerRef->UpdateBlackboard_State(EAIState::Idle);
	AIControllerRef->UpdateBlackboard_AttackRadius(GetAttackRadius());
	AIControllerRef->UpdateBlackboard_DefendRadius(GetDefendRadius());
	AIControllerRef->UpdateBlackboard_StartLocation(GetOwner()->GetActorLocation());
	AIControllerRef->UpdateBlackboard_MaxRandRadius(GetMaxRandRadius());
}

bool UProAIBehaviorsComponent::IsTriggerEnabled(ECombatTriggerFlags Trigger) const
{
	// Trigger는 1, 2, 4, 8... (bit mask) 형태이므로 그냥 마스킹
	const uint8 MaskValue = static_cast<uint8>(Trigger);
	return (CombatTriggerMask & MaskValue) != 0;
}

bool UProAIBehaviorsComponent::CanChangeState(EAIState ChangeState)
{
	// 현재 상태
	const EAIState PreviousState = AIControllerRef->GetPreviousState();
	const EAIState CurrentState = AIControllerRef->GetCurrentState();

	// 이미 죽어있거나 Dead면, Dead로만 전이 가능하다는 식으로 처리할 수도 있음
	if (IInterface_EnemyAI::Execute_IsDead(GetOwner()) || CurrentState == EAIState::Dead)
	{
		// 만약 죽은 뒤에는 다른 어떤 상태도 불가능하다면:
		return (ChangeState == EAIState::Dead);
	}

	// 상태별로 전용 함수 호출
	switch (ChangeState)
	{
	case EAIState::Dead:
		return CanChangeStateToDead();
	case EAIState::Combat:
		return CanChangeStateToCombat();
	case EAIState::Idle:
		return CanChangeStateToIdle();
	case EAIState::Seeking:
		return CanChangeStateToSeeking();
	case EAIState::Disabled:
		return CanChangeStateToDisabled();
	default:
		// 정의되지 않은 상태
		return false;
	}
}

bool UProAIBehaviorsComponent::UpdateState(EAIState NewState)
{
	// Blackboard에 실제 State 업데이트
	if (AIControllerRef)
	{
		AIControllerRef->UpdateBlackboard_State(NewState);
	}

	// 전이 후 실행할 로직
	switch (NewState)
	{
	case EAIState::Dead:
		HandleEnterDeadState();
		break;
	case EAIState::Combat:
		HandleEnterCombatState();
		break;
	case EAIState::Idle:
		HandleEnterIdleState();
		break;
	case EAIState::Seeking:
		HandleEnterSeekingState();
		break;
	case EAIState::Disabled:
		HandleEnterDisabledState();
		break;
	default:
		// 필요한 경우
		break;
	}

	return true;
}

bool UProAIBehaviorsComponent::CanChangeStateToDead() const
{
	// 실제로 사망했을 때만 Dead 상태로 갈 수 있다면
	return IInterface_EnemyAI::Execute_IsDead(GetOwner());
}

bool UProAIBehaviorsComponent::CanChangeStateToCombat() const
{
	// 전제조건: AttackableTargets가 비어있지 않아야
	if (AttackableTargets.IsEmpty())
		return false;

	if (AIControllerRef && AIControllerRef->GetCurrentState() == EAIState::Disabled)
		return false;


	return true;
}

bool UProAIBehaviorsComponent::CanChangeStateToIdle() const
{
	if (!AttackableTargets.IsEmpty())
		return false;

	return true;
}

bool UProAIBehaviorsComponent::CanChangeStateToSeeking() const
{
	// 이전 상태가 Combat이었다면 가능, Idle이었다면 불가능 등...
	if (!AIControllerRef)
		return false;

	const EAIState PreviousState = AIControllerRef->GetPreviousState();
	if (PreviousState == EAIState::Combat)
	{
		return true;
	}
	if (PreviousState == EAIState::Idle)
	{
		return false;
	}

	// 필요 시 다른 로직 처리
	return true;
}

bool UProAIBehaviorsComponent::CanChangeStateToDisabled() const
{
	// Disabled로 가는 특별 조건이 없다면 true
	return true;
}

// ========================================================================
// HandleEnterXXXState (Private Helpers)
// ========================================================================
void UProAIBehaviorsComponent::HandleEnterDeadState()
{
	// AIControllerRef->UpdateBlackboard_State(EAIState::Dead);
	// Dead 상태 진입 시 처리할 로직
	// ex) AIControllerRef->StopMovement(), Ragdoll 활성화, etc.
}

void UProAIBehaviorsComponent::HandleEnterCombatState()
{
	// AIControllerRef->UpdateBlackboard_State(EAIState::Combat);
	// Combat 상태 진입 시 해야 할 것들
	// SetStateAsAttacking();
	// 필요 시 추가 처리...
}

void UProAIBehaviorsComponent::HandleEnterIdleState()
{
	// Idle 상태 진입 시 처리
	// ex) StopMovement, Idle 애니메이션 등

	// AIControllerRef->UpdateBlackboard_State(EAIState::Idle);
}

void UProAIBehaviorsComponent::HandleEnterSeekingState()
{
	SetStateAsSeeking();
}

void UProAIBehaviorsComponent::HandleEnterDisabledState()
{
	// Disabled 상태 진입 시 처리
	// ex) 행동 불가, 입력 무시, 등
	AIControllerRef->UpdateBlackboard_State(EAIState::Disabled);
}

bool UProAIBehaviorsComponent::IsInCombat()
{
	return (AIControllerRef && AIControllerRef->GetCurrentState() == EAIState::Combat);
}

void UProAIBehaviorsComponent::SetStateAsSeeking()
{
	if (!AIControllerRef)
		return;

	AIControllerRef->UpdateBlackboard_PointOfInterest(RecentSenseHandle.LastKnownLocation);
	AIControllerRef->UpdateBlackboard_State(EAIState::Seeking);
}

void UProAIBehaviorsComponent::SetStateAsAttacking()
{
	// 공격 가능한 타겟 없으면
	if (AttackableTargets.IsEmpty())
	{
		AttackTarget = nullptr;
		AIControllerRef->UpdateBlackboard_AttackTarget(nullptr);

		if (CanChangeState(EAIState::Idle))
		{
			UpdateState(EAIState::Idle);
		}

		return;
	}

	// 1마리뿐이라면 바로 공격대상
	if (AttackableTargets.Num() == 1)
	{
		AttackTarget = AttackableTargets[0];
		AIControllerRef->UpdateBlackboard_AttackTarget(AttackTarget);
	}
	else
	{
		// 여러 개면, 거리 기반 가장 가까운 타겟 선정
		AActor* BestTarget = nullptr;
		float ShortestDistance = FLT_MAX;
		const FVector MyLocation = GetOwner()->GetActorLocation();

		for (AActor* PotentialTarget : AttackableTargets)
		{
			if (!IsValid(PotentialTarget))
				continue;

			const float Distance = FVector::Dist(MyLocation, PotentialTarget->GetActorLocation());
			if (Distance < ShortestDistance)
			{
				ShortestDistance = Distance;
				BestTarget = PotentialTarget;
			}
		}

		AttackTarget = BestTarget;
		AIControllerRef->UpdateBlackboard_AttackTarget(AttackTarget);
	}

	if (AIControllerRef->GetCurrentState() != EAIState::Combat)
	{
		if (CanChangeState(EAIState::Combat))
		{
			UpdateState(EAIState::Combat);
		}
	}
}

void UProAIBehaviorsComponent::ForceAttackTarget(AActor* NewActor)
{
	AttackableTargets.AddUnique(NewActor);
}

void UProAIBehaviorsComponent::HandlePerceptionUpdated(const FPerceivedActorInfo& PerceivedActorInfo)
{
	RecentSenseHandle = PerceivedActorInfo;

	switch (PerceivedActorInfo.DetectedSense)
	{
	case EAISense::Sight:
		if (PerceivedActorInfo.bCurrentlySensed)
			HandleSensedSight();
		else
			HandleLostSight();
		break;

	case EAISense::Hearing:
		if (PerceivedActorInfo.bCurrentlySensed)
			HandleSensedSound();
		else
			HandleLostSound();
		break;

	case EAISense::Damage:
		if (PerceivedActorInfo.bCurrentlySensed)
			HandleSensedDamage();
		else
			HandleLostDamage();
		break;

	default:
		break;
	}
}

void UProAIBehaviorsComponent::HandlePerceptionForgotten(const FPerceivedActorInfo& PerceivedActorInfo)
{
	// if (AttackableTargets.Contains(PerceivedActorInfo.DetectedActor))
	// {
	// 	AttackableTargets.Remove(PerceivedActorInfo.DetectedActor);
	// }
}

void UProAIBehaviorsComponent::HandleSensedSight()
{
	AActor* NewlySensedActor = RecentSenseHandle.DetectedActor;
	if (!NewlySensedActor) return;

	// 만약 ForgetTimers에 이미 타이머가 있으면 제거 (타겟을 다시 봤으니 Forget 취소)
	// if (ForgetTimers.Contains(NewlySensedActor))
	// {
	// 	GetWorld()->GetTimerManager().ClearTimer(ForgetTimers[NewlySensedActor]);
	// 	ForgetTimers.Remove(NewlySensedActor);
	// }

	// Attackable 목록에 추가
	AttackableTargets.AddUnique(NewlySensedActor);

	UpdateState(EAIState::Combat);
}

void UProAIBehaviorsComponent::HandleLostSight()
{
	// 예: 타겟을 일정 시간 후에 Forget(AttackableTargets에서 제거)하는 로직
}

void UProAIBehaviorsComponent::HandleSensedSound()
{
	AActor* NewlySensedActor = RecentSenseHandle.DetectedActor;
	if (!NewlySensedActor) return;

	AttackableTargets.AddUnique(NewlySensedActor);
}

void UProAIBehaviorsComponent::HandleSensedDamage()
{
	AActor* NewlySensedActor = RecentSenseHandle.DetectedActor;
	if (!NewlySensedActor) return;

	// 사운드나 시야와 달리 Damage 감지 시 곧바로 적대 타겟으로 인식할 수도 있고,
	// 혹은 특정 로직 상 Remove할 수도 있습니다. (현재 예시에서는 Remove)
	AttackableTargets.AddUnique(NewlySensedActor);
}

void UProAIBehaviorsComponent::HandleLostSound()
{
	// 예: 소리를 잃었을 때 로직
}

void UProAIBehaviorsComponent::HandleLostDamage()
{
	// 예: 데미지 감지가 사라졌을 때 로직
}

void UProAIBehaviorsComponent::RemoveActorFromAttackList(AActor* LostActor)
{
	if (!AttackableTargets.Contains(LostActor))
	{
		return;
	}

	// 완전히 제거
	AttackableTargets.Remove(LostActor);

	// ForgetTimers에도 있으면 클리어
	if (ForgetTimers.Contains(LostActor))
	{
		GetWorld()->GetTimerManager().ClearTimer(ForgetTimers[LostActor]);
		ForgetTimers.Remove(LostActor);
	}

	// 만약 현재 AttackTarget이 그 액터라면 해제
	if (AttackTarget == LostActor)
	{
		AttackTarget = nullptr;
	}
}
