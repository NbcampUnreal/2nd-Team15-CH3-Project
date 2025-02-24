#include "Abilities/Tasks/GSCTask_PlayMontageWaitForEvent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GSCLog.h"
#include "Abilities/Tasks/GSCAbilityTask_NetworkSyncPoint.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "Runtime/Launch/Resources/Version.h"


UGSCTask_PlayMontageWaitForEvent::UGSCTask_PlayMontageWaitForEvent(const FObjectInitializer& ObjectInitializer)
	: AnimRootMotionTranslationScale(0)
{
}

void UGSCTask_PlayMontageWaitForEvent::Activate()
{
	// 능력(Ability)이 유효하지 않으면 바로 종료합니다.
	if (!Ability)
	{
		return;
	}

	// 몽타주가 정상적으로 재생되었는지 여부를 나타내는 플래그
	bool bPlayedMontage = false;

	// AbilitySystemComponent의 유효성 검사 (엔진 버전에 따라 처리)
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	if (AbilitySystemComponent.IsValid())
#else
	if (AbilitySystemComponent)
#endif
	{
		// 현재 능력에 관련된 액터 정보를 가져옵니다.
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		// 액터의 애니메이션 인스턴스를 가져옵니다.
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			// 게임플레이 이벤트 델리게이트 바인딩: 지정된 EventTags와 일치하는 이벤트 발생 시 OnGameplayEvent() 호출
			EventHandle = AbilitySystemComponent->AddGameplayEventTagContainerDelegate(EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &UGSCTask_PlayMontageWaitForEvent::OnGameplayEvent));

			// 현재 몽타주의 섹션 남은 시간을 가져오지만, 이후 로직에서는 사용하지 않습니다.
			float CurrentMontageSectionTimeLeft = AbilitySystemComponent->GetCurrentMontageSectionTimeLeft();

			// 몽타주 재생 시도. PlayMontage 함수가 0보다 큰 값을 반환하면 재생 성공으로 간주합니다.
			if (AbilitySystemComponent->PlayMontage(Ability, Ability->GetCurrentActivationInfo(), MontageToPlay, Rate, StartSection) > 0.f)
			{
				// 만약 몽타주 재생 도중 콜백에 의해 능력이 취소되었으면, 더 이상 진행하지 않습니다.
				if (ShouldBroadcastAbilityTaskDelegates() == false)
				{
					return;
				}

				// 능력이 취소되었을 때 호출될 델리게이트를 바인딩합니다.
				CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject(this, &UGSCTask_PlayMontageWaitForEvent::OnAbilityCancelled);

				// 블렌드 아웃 상태 전환 시 호출될 델리게이트를 바인딩합니다.
				BlendingOutDelegate.BindUObject(this, &UGSCTask_PlayMontageWaitForEvent::OnMontageBlendingOut);
				AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, MontageToPlay);

				// 몽타주 종료 시 호출될 델리게이트를 바인딩합니다.
				MontageEndedDelegate.BindUObject(this, &UGSCTask_PlayMontageWaitForEvent::OnMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, MontageToPlay);

				// 캐릭터의 루트 모션 스케일을 설정하여, 애니메이션 재생에 따른 이동 효과를 조절합니다.
				ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
				if (Character && (Character->GetLocalRole() == ROLE_Authority ||
					(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
				{
					// 설정된 AnimRootMotionTranslationScale 값으로 캐릭터의 루트 모션을 적용합니다.
					Character->SetAnimRootMotionTranslationScale(AnimRootMotionTranslationScale);
				}

				// 몽타주가 정상적으로 재생되었음을 표시합니다.
				bPlayedMontage = true;
			}
		}
		else
		{
			// 애니메이션 인스턴스가 유효하지 않으므로 몽타주 재생 실패. 경고 로그 출력 (한글 메시지)
			GSC_LOG(Warning, TEXT("UGSCTask_PlayMontageWaitForEvent: PlayMontage 호출 실패! (AnimInstance가 nullptr)"));
		}
	}
	else
	{
		// AbilitySystemComponent가 유효하지 않은 경우 경고 로그 출력 (한글 메시지)
		GSC_LOG(Warning, TEXT("UGSCTask_PlayMontageWaitForEvent: 유효하지 않은 AbilitySystemComponent 사용 중"));
	}

	// 만약 몽타주 재생이 실패했다면, 취소 델리게이트를 통해 실패를 알립니다.
	if (!bPlayedMontage)
	{
		GSC_LOG(Warning, TEXT("UGSCTask_PlayMontageWaitForEvent: Ability '%s'에서 몽타주 '%s' 재생 실패; Task Instance 이름: '%s'"),
		        *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	// 아바타(캐릭터)를 대상으로 대기 상태로 전환합니다.
	SetWaitingOnAvatar();
}

void UGSCTask_PlayMontageWaitForEvent::ExternalCancel()
{
	// AbilitySystemComponent 유효성 검사 (엔진 버전에 따라 처리)
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	check(AbilitySystemComponent.IsValid());
#else
    check(AbilitySystemComponent);
#endif

	// 능력 취소 처리를 수행합니다.
	OnAbilityCancelled();
	// 상위 클래스의 ExternalCancel() 호출
	Super::ExternalCancel();
}

FString UGSCTask_PlayMontageWaitForEvent::GetDebugString() const
{
	const UAnimMontage* PlayingMontage = nullptr;
	if (Ability)
	{
		// 현재 능력에 관련된 액터 정보를 가져옵니다.
		const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		// 액터의 애니메이션 인스턴스를 가져옵니다.
		const UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			// 현재 재생 중인 몽타주가 MontageToPlay와 일치하면 해당 몽타주를, 그렇지 않으면 현재 활성화된 몽타주를 사용합니다.
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay.Get() : AnimInstance->GetCurrentActiveMontage();
		}
	}

	// 재생 중인 몽타주의 이름과 현재 활성화된 몽타주의 이름을 문자열로 포맷하여 반환합니다.
	return FString::Printf(TEXT("PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s"),
	                       *GetNameSafe(MontageToPlay), *GetNameSafe(PlayingMontage));
}

void UGSCTask_PlayMontageWaitForEvent::OnDestroy(const bool AbilityEnded)
{
	// 주석: 몽타주 종료 델리게이트는 멀티캐스트가 아니므로, 다음 몽타주 재생 시 자동으로 해제됩니다.
	// 다만, 멀티캐스트 델리게이트는 명시적으로 해제해야 합니다.
	if (Ability)
	{
		// 능력 취소 이벤트 핸들을 제거합니다.
		Ability->OnGameplayAbilityCancelled.Remove(CancelledHandle);
		// 만약 능력이 종료되었고, bStopWhenAbilityEnds가 true이면 몽타주 중단을 수행합니다.
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	// AbilitySystemComponent의 유효성을 확인한 후, 이벤트 델리게이트를 해제합니다.
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	if (AbilitySystemComponent.IsValid())
#else
	if (AbilitySystemComponent)
#endif
	{
		AbilitySystemComponent->RemoveGameplayEventTagContainerDelegate(EventTags, EventHandle);
	}

	// 상위 클래스의 OnDestroy() 호출
	Super::OnDestroy(AbilityEnded);
}


void UGSCTask_PlayMontageWaitForEvent::UnbindAllDelegate()
{
	// 모든 관련 델리게이트를 클리어합니다.
	OnCompleted.Clear();
	OnBlendOut.Clear();
	OnInterrupted.Clear();
}

UGSCTask_PlayMontageWaitForEvent* UGSCTask_PlayMontageWaitForEvent::PlayMontageAndWaitForEvent(
	UGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	UAnimMontage* MontageToPlay,
	FGameplayTagContainer EventTags,
	float Rate,
	FName StartSection,
	bool bStopWhenAbilityEnds,
	float AnimRootMotionTranslationScale)
{
	// 글로벌 능력 스케일러를 적용하여 몽타주 재생 속도를 조절합니다.
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate(Rate);

	// OwningAbility가 nullptr인 경우, 오류 로그를 출력하고 nullptr를 반환하여 크래시를 방지합니다.
	if (!OwningAbility)
	{
		GSC_LOG(Error, TEXT("UGSCTask_PlayMontageWaitForEvent: 소유하는 Ability이 null입니다. 이런 상황은 발생하면 안됩니다."));
		return nullptr;
	}

	// 새로운 태스크 인스턴스를 생성합니다.
	UGSCTask_PlayMontageWaitForEvent* MyObj = NewAbilityTask<UGSCTask_PlayMontageWaitForEvent>(OwningAbility, TaskInstanceName);
	// 생성된 태스크 인스턴스의 멤버 변수들을 초기화합니다.
	MyObj->MontageToPlay = MontageToPlay;
	MyObj->EventTags = EventTags;
	MyObj->Rate = Rate;
	MyObj->StartSection = StartSection;
	MyObj->AnimRootMotionTranslationScale = AnimRootMotionTranslationScale;
	MyObj->bStopWhenAbilityEnds = bStopWhenAbilityEnds;

	return MyObj;
}

bool UGSCTask_PlayMontageWaitForEvent::StopPlayingMontage() const
{
	// 현재 능력과 관련된 액터 정보를 가져옵니다.
	const FGameplayAbilityActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return false;
	}

	// 액터의 애니메이션 인스턴스를 가져옵니다.
	const UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return false;
	}

	// AbilitySystemComponent와 Ability가 유효한지 확인합니다.
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
	if (AbilitySystemComponent.IsValid() && Ability)
#else
    if (AbilitySystemComponent && Ability)
#endif
	{
		// 현재 재생 중인 몽타주가 Ability에 의해 재생되고 있으며, MontageToPlay와 동일한지 확인합니다.
		if (AbilitySystemComponent->GetAnimatingAbility() == Ability && AbilitySystemComponent->GetCurrentMontage() == MontageToPlay)
		{
			// 현재 재생 중인 몽타주 인스턴스를 가져와서, 관련 델리게이트들을 해제합니다.
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			// AbilitySystemComponent를 통해 현재 재생 중인 몽타주를 중단합니다.
			AbilitySystemComponent->CurrentMontageStop();
			return true;
		}
	}

	return false;
}

void UGSCTask_PlayMontageWaitForEvent::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted) const
{
	// 현재 능력에서 재생 중인 몽타주가 MontageToPlay와 일치하는지 확인합니다.
	if (Ability && Ability->GetCurrentMontage() == MontageToPlay)
	{
		if (Montage == MontageToPlay)
		{
			// AbilitySystemComponent의 현재 애니메이팅 능력을 클리어합니다.
			AbilitySystemComponent->ClearAnimatingAbility(Ability);

			// 캐릭터의 루트 모션 스케일을 기본값(1.0)으로 재설정합니다.
			ACharacter* Character = Cast<ACharacter>(GetAvatarActor());
			if (Character && (Character->GetLocalRole() == ROLE_Authority ||
				(Character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted)))
			{
				Character->SetAnimRootMotionTranslationScale(1.f);
			}
		}
	}

	// 인터럽트 여부에 따라 각각의 델리게이트를 호출합니다.
	if (bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnBlendOut.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UGSCTask_PlayMontageWaitForEvent::OnAbilityCancelled() const
{
	// 현재 재생 중인 몽타주가 존재하면 중단하고, 취소 델리게이트를 호출합니다.
	if (StopPlayingMontage())
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCancelled.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}
}

void UGSCTask_PlayMontageWaitForEvent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	// 만약 인터럽트 없이 정상적으로 재생이 완료되었다면, 완료 델리게이트 호출
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast(FGameplayTag(), FGameplayEventData());
		}
	}

	// 태스크 종료 처리
	EndTask();
}

void UGSCTask_PlayMontageWaitForEvent::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) const
{
	// 만약 델리게이트를 방송할 수 없는 상태라면 처리를 중단합니다.
	if (!ShouldBroadcastAbilityTaskDelegates())
	{
		return;
	}

	// Payload 데이터를 복사하여 임시 데이터 변수에 저장한 후, 이벤트 태그를 설정합니다.
	FGameplayEventData TempData = *Payload;
	TempData.EventTag = EventTag;

	// 서버 동기화를 위한 태스크를 생성합니다. OnlyServerWait 모드를 사용하여 서버 준비가 완료될 때까지 대기합니다.
	UGSCAbilityTask_NetworkSyncPoint* Task = UGSCAbilityTask_NetworkSyncPoint::WaitNetSync(Ability, EGSCAbilityTaskNetSyncType::OnlyServerWait);

	// 서버 동기화가 완료되면 OnServerSyncEventReceived()가 호출되도록 델리게이트를 바인딩합니다.
	Task->OnSyncDelegate.AddUObject(this, &UGSCTask_PlayMontageWaitForEvent::OnServerSyncEventReceived, EventTag, TempData);
	// 동기화 태스크를 활성화합니다.
	Task->ReadyForActivation();
}

void UGSCTask_PlayMontageWaitForEvent::OnServerSyncEventReceived(const FGameplayTag EventTag, const FGameplayEventData EventData) const
{
	// 만약 델리게이트를 방송할 수 있는 상태라면, EventReceived 델리게이트를 호출하여 이벤트를 전달합니다.
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		EventReceived.Broadcast(EventTag, EventData);
	}
}
