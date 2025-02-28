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
	FGameplayTag CurrentState = AIControllerRef->GetCurrentStateTag();
	FGameplayTag PreviousState = AIControllerRef->GetPreviousStateTag();

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

bool UAIBehaviorsComponent::UpdateState(FGameplayTag UpdateState)
{
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

	return true;
}

bool UAIBehaviorsComponent::IsInCombat()
{
	FGameplayTag CurrentState = AIControllerRef->GetCurrentStateTag();
	return CurrentState == AIGameplayTags::AIState_Combat;
}

void UAIBehaviorsComponent::HandleSensedSight()
{
	AActor* NewlySensedActor = RecentSenseHandle.DetectedActor;
	if (!NewlySensedActor)
		return;

	// 1) 만약 ForgetTimers에 이미 등록된 타이머(“잃어버린 타깃”)가 있다면 클리어
	if (ForgetTimers.Contains(NewlySensedActor))
	{
		GetWorld()->GetTimerManager().ClearTimer(ForgetTimers[NewlySensedActor]);
		ForgetTimers.Remove(NewlySensedActor);
	}

	// 2) AttackableTargets에 추가 (기존 코드)
	AttackableTargets.AddUnique(NewlySensedActor);

	// 3) 이미 AttackTarget이 이 액터라면 별도 처리를 안 해도 되고,
	//    AttackTarget이 없으면 새로 할당하는 등 필요 로직을 넣으시면 됩니다.
	// ...


	// // 이미 공격 대상이 설정되어 있다면, 추가적인 타겟 설정을 하지 않음
	// if (AttackTarget == RecentSenseHandle.DetectedActor)
	// 	return;
	//
	// ECombatTriggerFlags SightCombatTrigger = ECombatTriggerFlags::Sight;
	// if (!IsTriggerEnabled(SightCombatTrigger))
	// 	return;
}

void UAIBehaviorsComponent::HandleLostSight()
{
	AActor* LostActor = RecentSenseHandle.DetectedActor;
	if (!LostActor)
		return;

	// 이미 AttackableTargets에 들어있는 Actor라면,
	// 곧바로 제거가 아니라, 일정 시간 뒤에 제거하는 타이머를 건다.
	if (AttackableTargets.Contains(LostActor))
	{
		// 혹시 이전에 설정된 타이머가 있으면 초기화
		if (ForgetTimers.Contains(LostActor))
		{
			GetWorld()->GetTimerManager().ClearTimer(ForgetTimers[LostActor]);
			ForgetTimers.Remove(LostActor);
		}

		// 일정 시간이 지난 후 RemoveActorFromAttackList를 호출하는 타이머
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle,
		                                       FTimerDelegate::CreateUObject(this, &UAIBehaviorsComponent::RemoveActorFromAttackList, LostActor),
		                                       ForgetSightTime, false);
		ForgetTimers.Add(LostActor, TimerHandle);

		// 또한, “AI가 눈 앞에서 사라졌지만, 아직 공격 대상으로 처리 중” 이므로
		// AI가 “Seeking” 상태로 전환하도록 할 수도 있음.
		//  e.g.  UpdateState(AIGameplayTags::AIState_Seeking);
	}
}

void UAIBehaviorsComponent::HandleSensedSound()
{
	// EAIState CurrentState = GetCurrentState();
	// if (CurrentState == EAIState::Idle || CurrentState == EAIState::Investigating || CurrentState == EAIState::Seeking)
	// {
	// 	SetStateAsInvestigating(Location);
	// }
}

void UAIBehaviorsComponent::HandleSensedDamage()
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

void UAIBehaviorsComponent::HandleLostSound()
{
}

void UAIBehaviorsComponent::HandleLostDamage()
{
}

void UAIBehaviorsComponent::SetStateAsSeeking()
{
	AIControllerRef->UpdateBlackboard_PointOfInterest(RecentSenseHandle.LastKnownLocation);
	AIControllerRef->UpdateBlackboard_State(AIGameplayTags::AIState_Seeking);
	// UKismetSystemLibrary::DrawDebugSphere(this, RecentSenseHandle.LastKnownLocation, 150.0f, 12, FLinearColor::Red, 15.0f, 10.0f);
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
	RecentSenseHandle = PerceivedActorInfo;

	if (RecentSenseHandle.DetectedSense == EAISense::Sight)
	{
		if (RecentSenseHandle.bCurrentlySensed)
			HandleSensedSight();
		else
			HandleLostSight();
	}

	else if (RecentSenseHandle.DetectedSense == EAISense::Hearing)
	{
		if (RecentSenseHandle.bCurrentlySensed)
			HandleSensedSound();
		else
			HandleSensedSound();
	}

	else if (RecentSenseHandle.DetectedSense == EAISense::Damage)
	{
		if (RecentSenseHandle.bCurrentlySensed)
			HandleSensedDamage();
		else
			HandleLostDamage();
	}
}

void UAIBehaviorsComponent::HandlePerceptionForgotten(const FPerceivedActorInfo& PerceivedActorInfo)
{
	if (AttackableTargets.Contains(PerceivedActorInfo.DetectedActor))
		AttackableTargets.Remove(PerceivedActorInfo.DetectedActor);
}

void UAIBehaviorsComponent::RemoveActorFromAttackList(AActor* LostActor)
{
	AI_ENEMY_SCREEN_LOG_ERROR(5.0f, "RemoveActorFromAttackList 함수가 호출 1번이 되어야 합니다");
	// 이미 Target이 다시 보이는 상황에서 Memory를 지울 수도 있으므로 체크
	if (!AttackableTargets.Contains(LostActor))
	{
		return;
	}

	// 이 시점까지 재감지(시야 회복)하지 않았다면, 완전히 제거
	AttackableTargets.Remove(LostActor);

	// 타이머 맵에서도 제거
	if (ForgetTimers.Contains(LostActor))
	{
		GetWorld()->GetTimerManager().ClearTimer(ForgetTimers[LostActor]);
		ForgetTimers.Remove(LostActor);
	}

	// 만약 AttackTarget == LostActor 이었다면, nullptr로 세팅
	if (AttackTarget == LostActor)
	{
		AttackTarget = nullptr;
	}

	// 더이상 공격 대상이 없으면, Idle로 바꿀 수도 있고
	//  UpdateState(AIGameplayTags::AIState_Idle); 
}
