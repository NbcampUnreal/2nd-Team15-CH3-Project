// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Abilities/GameplayAbility_EnemyMeleeBase.h"

#include "MotionWarpingComponent.h"
#include "Abilities/Tasks/GSCTask_PlayMontageWaitForEvent.h"
#include "AI/EnemyAIBase.h"
#include "AI/Components/AIBehaviorsComponent.h"
#include "GameFramework/Character.h"


UGameplayAbility_EnemyMeleeBase::UGameplayAbility_EnemyMeleeBase()
{
	// 여기서 기본값 세팅(예: bActivateOnGranted 등)
	// bActivateOnGranted = false; 
}

void UGameplayAbility_EnemyMeleeBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 1) 커밋 (비용/쿨다운)
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 2) 공격 몽타주가 없으면 즉발 로직으로 대체 가능
	if (!AttackMontage)
	{
		// 즉발 시 AttackEffectTag로 ApplyEffectContainer등 처 리 가능
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// -- 여기서 Blueprint 쪽 SetupMotionWarpTarget을 호출할 수도 있고,
	//    또는 OnAttackEventReceived("Event.Warp") 시점에 호출하도록 디자인할 수도 있음.
	//    필요하다면 여기서:
	//    SetupMotionWarpTarget(CachedTargetActor.Get());

	// 3) 몽타주 재생 + 이벤트 대기
	UGSCTask_PlayMontageWaitForEvent* MontageTask = UGSCTask_PlayMontageWaitForEvent::PlayMontageAndWaitForEvent(
		this,
		NAME_None,
		AttackMontage,
		EventTags, // Warp/Hit 등
		MontagePlayRate,
		NAME_None, // section
		true, // bStopWhenAbilityEnds
		1.0f
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 콜백 바인딩
	MontageTask->OnBlendOut.AddDynamic(this, &UGameplayAbility_EnemyMeleeBase::OnMontageCompleted);
	MontageTask->OnCompleted.AddDynamic(this, &UGameplayAbility_EnemyMeleeBase::OnMontageCompleted);

	MontageTask->OnInterrupted.AddDynamic(this, &UGameplayAbility_EnemyMeleeBase::OnMontageInterruptedOrCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UGameplayAbility_EnemyMeleeBase::OnMontageInterruptedOrCancelled);

	MontageTask->EventReceived.AddDynamic(this, &UGameplayAbility_EnemyMeleeBase::OnAttackEventReceived);

	MontageTask->ReadyForActivation();
}

void UGameplayAbility_EnemyMeleeBase::OnMontageInterruptedOrCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// 몽타주 중단 시, Warp 타겟들도 제거 (원한다면)
	if (ACharacter* EnemyCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (auto* MotionWarpComp = EnemyCharacter->FindComponentByClass<UMotionWarpingComponent>())
		{
			for (const FName& WarpName : WarpTargetNames)
			{
				MotionWarpComp->RemoveWarpTarget(WarpName);
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGameplayAbility_EnemyMeleeBase::OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// 몽타주가 정상 완료된 시점에 Warp 타겟 제거
	if (ACharacter* EnemyCharacter = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (auto* MotionWarpComp = EnemyCharacter->FindComponentByClass<UMotionWarpingComponent>())
		{
			for (const FName& WarpName : WarpTargetNames)
			{
				MotionWarpComp->RemoveWarpTarget(WarpName);
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGameplayAbility_EnemyMeleeBase::OnAttackEventReceived(FGameplayTag InEventTag, FGameplayEventData EventData)
{
	if (InEventTag == WarpEventTag)
	{
		if (AEnemyAIBase* EnemyAvatar = Cast<AEnemyAIBase>(GetCurrentActorInfo()->AvatarActor))
		{
			if (AActor* TargetActor = EnemyAvatar->AIBehaviorsComponent->AttackTarget)
				OnSetupMotionWarpTarget(TargetActor);
		}
	}
	// 만약 "Event.Hit" 태그면 공격 피해 적용
	else if (InEventTag == HitEventTag)
	{
		if (AttackEffectTag.IsValid())
		{
			ApplyEffectContainer(AttackEffectTag, EventData);
		}
	}
}
