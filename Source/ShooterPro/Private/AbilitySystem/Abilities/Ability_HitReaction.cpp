// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Ability_HitReaction.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UAbility_HitReaction::UAbility_HitReaction()
{
	// Ability 기본 프로퍼티 설정 (예시)
	// InstancingPolicy: 인스턴스화된 어빌리티 (주로 피격 등 일회성 이벤트에 사용)
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;

	// Triggers(가령 Hit 이벤트가 왔을 때 Activate) - Blueprint 등에서 세팅할 수도 있음
	/*
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.HitReaction")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	*/
}

void UAbility_HitReaction::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 1) Super 호출 (기본 체크 및 Commit)
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 2) 혹시 Montage가 세팅되지 않았으면 어빌리티 종료
	if (!HitMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 3) Montage 재생을 위한 AbilityTask 생성
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None, // TaskInstanceName
		HitMontage,
		1.f, // Rate
		NAME_None, // StartSection
		false, // bStopWhenAbilityEnds
		1.f, // AnimRootMotionTranslationScale
		0.f // StartTimeSeconds
	);

	// 4) 콜백 바인딩: 완료/취소/Interrupted/BlendOut 시점
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UAbility_HitReaction::OnMontageFinished);
		MontageTask->OnBlendOut.AddDynamic(this, &UAbility_HitReaction::OnMontageFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &UAbility_HitReaction::OnMontageCancelled);
		MontageTask->OnCancelled.AddDynamic(this, &UAbility_HitReaction::OnMontageCancelled);

		MontageTask->ReadyForActivation();
	}

	// 5) 어빌리티를 커밋 (쿨타임/코스트 소모 등)
	CommitAbility(Handle, ActorInfo, ActivationInfo);
}

void UAbility_HitReaction::OnMontageFinished()
{
	// Montage가 정상 종료되거나 BlendOut된 경우
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, false, false);
}

void UAbility_HitReaction::OnMontageCancelled()
{
	// Montage가 Interrupted되거나 Cancelled된 경우
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
