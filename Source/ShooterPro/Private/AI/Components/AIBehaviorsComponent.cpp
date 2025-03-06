#include "AI/Components/AIBehaviorsComponent.h"

#include "ProGmaeplayTag.h"
#include "AI/AIGameplayTags.h"
#include "AI/EnemyAIBase.h"
#include "AI/EnemyAIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"


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
		AIControllerRef->GetDetectionInfoManager()->OnAddPerceptionUpdated.AddDynamic(this, &UAIBehaviorsComponent::HandlePerceptionUpdated);
		AIControllerRef->GetDetectionInfoManager()->OnRemoveExpiredDetection.AddDynamic(this, &UAIBehaviorsComponent::HandlePerceptionForgotten);
	}
}

void UAIBehaviorsComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AIControllerRef->GetDetectionInfoManager()->OnAddPerceptionUpdated.RemoveAll(this);
	AIControllerRef->GetDetectionInfoManager()->OnRemoveExpiredDetection.RemoveAll(this);
	Super::EndPlay(EndPlayReason);
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

	if (ChangeState == AIGameplayTags::AIState_Dead)
	{
		return IInterface_EnemyAI::Execute_IsDead(GetOwner());
	}

	if (IInterface_EnemyAI::Execute_IsDead(GetOwner()) || CurrentState == AIGameplayTags::AIState_Dead)
		return false;

	//원하는 상태가 컴뱃이라면
	if (ChangeState == AIGameplayTags::AIState_Combat)
	{
		//내 현재상태가 죽음이 아니고 && 공격가능한 대상들이 하나라도 있다면
		if (!AttackableTargets.IsEmpty())
		{
			return true;
		}

		//비활성화 모드 일 때
		if (CurrentState == AIGameplayTags::AIState_Disabled)
		{
			IGameplayTagAssetInterface* TagAsset = Cast<IGameplayTagAssetInterface>(CharacterRef);
			if (TagAsset && TagAsset->HasMatchingGameplayTag(ProGameplayTags::Ability_HitReact))
			{
				return true;
			}

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

void UAIBehaviorsComponent::ForceAttackTarget(AActor* NewActor)
{
	AttackableTargets.AddUnique(NewActor);
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

	AttackableTargets.AddUnique(NewlySensedActor);
}

void UAIBehaviorsComponent::HandleLostSight()
{
}

void UAIBehaviorsComponent::HandleSensedSound()
{
	AActor* NewlySensedActor = RecentSenseHandle.DetectedActor;
	AttackableTargets.AddUnique(NewlySensedActor);
}

void UAIBehaviorsComponent::HandleSensedDamage()
{
	AActor* NewlySensedActor = RecentSenseHandle.DetectedActor;
	AttackableTargets.Remove(NewlySensedActor);
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
}

void UAIBehaviorsComponent::SetStateAsAttacking()
{
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

	//2마리 이상일 경우

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
			HandleLostSound();
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
