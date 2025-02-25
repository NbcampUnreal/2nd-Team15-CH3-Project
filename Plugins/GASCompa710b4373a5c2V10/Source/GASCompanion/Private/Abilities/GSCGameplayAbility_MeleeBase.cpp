#include "Abilities/GSCGameplayAbility_MeleeBase.h"

#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Abilities/Tasks/GSCTask_PlayMontageWaitForEvent.h"
#include "Components/GSCComboManagerComponent.h"

UGSCGameplayAbility_MeleeBase::UGSCGameplayAbility_MeleeBase()
{
	// 기본 생성자에서는 추가 초기화 작업이 필요하지 않습니다.
}

void UGSCGameplayAbility_MeleeBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	// 능력 비용을 커밋합니다. 실패하면 능력을 종료합니다.
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// Commit 실패 시, 능력을 강제 종료 (취소 플래그 true, 매끄럽게 종료되지 않음)
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 아바타 액터를 검색합니다.
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!AvatarActor)
	{
		// 아바타 액터가 유효하지 않으면 능력을 종료합니다.
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 아바타 액터에서 콤보 매니저 컴포넌트를 검색하여 할당합니다.
	ComboManagerComponent = UGSCBlueprintFunctionLibrary::GetComboManagerComponent(AvatarActor);
	if (!ComboManagerComponent)
	{
		// 콤보 매니저 컴포넌트가 없으면 능력을 종료합니다.
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 콤보 매니저의 콤보 카운트를 증가시킵니다.
	ComboManagerComponent->IncrementCombo();

	// 다음 콤보 애니메이션 몽타주를 선택합니다.
	UAnimMontage* Montage = GetNextComboMontage();

	// 몽타주 재생 및 이벤트 대기를 위한 태스크를 생성합니다.
	UGSCTask_PlayMontageWaitForEvent* Task = UGSCTask_PlayMontageWaitForEvent::PlayMontageAndWaitForEvent(
		this, // 소유하는 능력 (this)
		NAME_None, // 태스크 인스턴스 이름 (기본값 사용)
		Montage, // 재생할 몽타주
		WaitForEventTag, // 이벤트 태그 컨테이너: 지정된 태그와 일치하는 이벤트 수신 시 효과 적용
		Rate, // 몽타주 재생 속도
		NAME_None, // 시작 섹션 이름 (없음)
		true, // 능력 종료 시 몽타주 중단 여부
		1.0f // 루트 모션 스케일 (기본값 1.0f)
	);

	// 태스크의 각 델리게이트에 콜백 함수를 바인딩합니다.
	// OnBlendOut과 OnCompleted: 몽타주 완료 시 호출되어 능력을 종료합니다.
	Task->OnBlendOut.AddDynamic(this, &UGSCGameplayAbility_MeleeBase::OnMontageCompleted);
	Task->OnCompleted.AddDynamic(this, &UGSCGameplayAbility_MeleeBase::OnMontageCompleted);

	// OnInterrupted와 OnCancelled: 몽타주가 중단되거나 취소된 경우 호출되어 능력을 종료합니다.
	Task->OnInterrupted.AddDynamic(this, &UGSCGameplayAbility_MeleeBase::OnMontageCancelled);
	Task->OnCancelled.AddDynamic(this, &UGSCGameplayAbility_MeleeBase::OnMontageCancelled);

	// EventReceived: 특정 이벤트 발생 시 효과 컨테이너를 적용합니다.
	Task->EventReceived.AddDynamic(this, &UGSCGameplayAbility_MeleeBase::OnEventReceived);

	// 태스크를 활성화합니다.
	Task->ReadyForActivation();
}

void UGSCGameplayAbility_MeleeBase::OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGSCGameplayAbility_MeleeBase::OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGSCGameplayAbility_MeleeBase::OnEventReceived(const FGameplayTag EventTag, const FGameplayEventData EventData)
{
	// 이벤트 태그에 해당하는 효과 컨테이너를 적용합니다.
	ApplyEffectContainer(EventTag, EventData);
}

UAnimMontage* UGSCGameplayAbility_MeleeBase::GetNextComboMontage()
{
	// 콤보 매니저 컴포넌트가 유효하지 않으면 nullptr 반환
	if (!ComboManagerComponent)
	{
		return nullptr;
	}

	// 현재 콤보 인덱스를 가져옵니다.
	int32 ComboIndex = ComboManagerComponent->ComboIndex;

	// 콤보 인덱스가 몽타주 배열의 범위를 초과하면 0으로 리셋합니다.
	if (ComboIndex >= Montages.Num())
	{
		ComboIndex = 0;
	}

	// 배열 인덱스가 유효한지 확인 후 해당 몽타주를 반환, 유효하지 않으면 nullptr 반환
	return Montages.IsValidIndex(ComboIndex) ? Montages[ComboIndex] : nullptr;
}
