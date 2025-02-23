#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask.h"
#include "Animation/AnimMontage.h"
#include "GSCTask_PlayMontageWaitForEvent.generated.h"

/*
    이 델리게이트는 두 개의 파라미터(게임플레이 태그와 이벤트 데이터를 전달)를 받으며,
    애니메이션 몽타주 재생 도중 혹은 이벤트 발생 시 호출되는 콜백을 처리하기 위해 사용됩니다.
*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGSCPlayMontageAndWaitForEventDelegate, FGameplayTag, EventTag, FGameplayEventData, EventData);

/**
 * UGSCTask_PlayMontageWaitForEvent
*
 * 이 클래스는 애니메이션 몽타주(연출)를 재생하면서 동시에 특정 게임플레이 이벤트를 대기하는  능력 태스크(Ability Task)를 구현합니다.
*
 * 주요 기능:
 * - 지정된 몽타주를 재생하며, 재생 중 발생하는 다양한 상태(블렌드 아웃, 완료, 인터럽트 등)를 처리합니다.
 * - 지정된 게임플레이 태그(EventTags)와 일치하는 이벤트가 발생하면, 해당 이벤트와 데이터를 델리게이트를 통해 전달합니다.
 * - 능력 종료나 외부 취소 시, 자동으로 몽타주 재생을 중단하는 기능을 제공합니다.
*
 * 사용 예:
 * - 근접 공격 콤보 등 복합적인 애니메이션과 이벤트 처리가 필요한 게임 플레이 상황에서 활용할 수 있습니다.
*/
UCLASS()
class GASCOMPANION_API UGSCTask_PlayMontageWaitForEvent : public UAbilityTask
{
	GENERATED_BODY()

public:
	// 생성자: 객체 초기화 및 기본 멤버 변수 설정
	UGSCTask_PlayMontageWaitForEvent(const FObjectInitializer& ObjectInitializer);

	// 태스크가 활성화될 때 호출되며, 몽타주 재생 및 이벤트 바인딩 등의 초기화 작업을 수행합니다.
	virtual void Activate() override;

	// 외부에서 태스크를 취소할 때 호출되며, 취소에 따른 후처리를 수행합니다.
	virtual void ExternalCancel() override;

	// 디버그 정보를 문자열 형태로 반환하여, 현재 재생 중인 몽타주 등의 상태를 확인할 수 있도록 합니다.
	virtual FString GetDebugString() const override;

	// 태스크 종료 시 호출되며, 델리게이트 해제 및 리소스 정리 작업을 수행합니다.
	virtual void OnDestroy(bool AbilityEnded) override;

	/** 
	 * 몽타주가 완전히 재생을 마쳤을 때 호출되는 델리게이트.
	 * 예를 들어, 공격 애니메이션이 정상적으로 완료된 경우에 실행됩니다.
	 */
	UPROPERTY(BlueprintAssignable)
	FGSCPlayMontageAndWaitForEventDelegate OnCompleted;

	/** 
	 * 몽타주가 블렌드 아웃 상태로 전환될 때 호출되는 델리게이트.
	 * 블렌드 아웃은 몽타주가 서서히 사라지는 애니메이션 효과를 의미합니다.
	 */
	UPROPERTY(BlueprintAssignable)
	FGSCPlayMontageAndWaitForEventDelegate OnBlendOut;

	/** 
	 * 몽타주가 인터럽트되어 중단될 경우 호출되는 델리게이트.
	 * 다른 애니메이션이 재생되어 현재 몽타주가 강제로 중단되는 상황에 사용됩니다.
	 */
	UPROPERTY(BlueprintAssignable)
	FGSCPlayMontageAndWaitForEventDelegate OnInterrupted;

	/** 
	 * 다른 능력이나 외부 요인에 의해 태스크가 명시적으로 취소되었을 때 호출되는 델리게이트.
	 */
	UPROPERTY(BlueprintAssignable)
	FGSCPlayMontageAndWaitForEventDelegate OnCancelled;

	/** 
	 * 지정된 게임플레이 이벤트가 발생했을 때 호출되는 델리게이트.
	 * 이벤트 태그와 이벤트 데이터가 함께 전달되어, 이후 로직에서 활용할 수 있습니다.
	 */
	UPROPERTY(BlueprintAssignable)
	FGSCPlayMontageAndWaitForEventDelegate EventReceived;

	/**
	 * 모든 애니메이션 델리게이트를 해제하는 함수입니다.
	 * (단, OnCancelled 델리게이트는 제외)
	 *
	 * 사용 예:
	 * - 태스크가 더 이상 이벤트를 처리하지 않아야 할 때 호출하여, 불필요한 콜백 호출을 방지합니다.
	*/
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks")
	void UnbindAllDelegate();

	/**
	 * 몽타주를 재생하고, 해당 몽타주가 종료될 때까지 대기하는 정적 함수입니다.
	 * 만약 지정된 게임플레이 이벤트(또는 모든 이벤트)가 발생하면, EventReceived 델리게이트가 호출됩니다.
	*
	 * 파라미터:
	 * @param OwningAbility			- 이 태스크를 소유하는 능력 객체 (UGameplayAbility)
	 * @param TaskInstanceName		- 태스크 인스턴스의 이름 (추후 식별에 사용)
	 * @param MontageToPlay			- 재생할 애니메이션 몽타주 (UAnimMontage)
	 * @param EventTags				- 감지할 게임플레이 이벤트 태그 컨테이너. 빈 경우 모든 이벤트 허용.
	 * @param Rate					- 몽타주 재생 속도 (기본값: 1.0f)
	 * @param StartSection			- 재생 시작할 몽타주 섹션의 이름 (기본값: 없음)
	 * @param bStopWhenAbilityEnds	- 능력 종료 시 몽타주를 중단할지 여부 (true이면 중단)
	 * @param AnimRootMotionTranslationScale - 루트 모션 이동에 적용할 스케일 (모션 차단은 0)
	*
	 * 동작 방식:
	 * - 지정된 능력 객체를 기반으로 새로운 태스크 인스턴스를 생성합니다.
	 * - 몽타주 재생 및 이벤트 바인딩을 위한 초기 변수들을 설정합니다.
	*/
	UFUNCTION(BlueprintCallable, Category= "Ability|GAS Companion|Tasks", meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
	static UGSCTask_PlayMontageWaitForEvent* PlayMontageAndWaitForEvent(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		UAnimMontage* MontageToPlay,
		FGameplayTagContainer EventTags,
		float Rate = 1.f,
		FName StartSection = NAME_None,
		bool bStopWhenAbilityEnds = true,
		float AnimRootMotionTranslationScale = 1.f);

private:
	/**
	 * 현재 재생 중인 몽타주가 존재하는지 확인하고, 존재할 경우 중단하는 함수입니다.
	 * 반환값: 몽타주가 중단되면 true, 그렇지 않으면 false를 반환합니다.
	 *
	 * 이 함수는 능력 종료나 인터럽트 상황에서 불필요한 애니메이션 재생을 방지하기 위해 호출됩니다.
	 */
	bool StopPlayingMontage() const;

	/** 
	 * 몽타주가 블렌드 아웃 상태로 전환될 때 호출되는 콜백 함수입니다.
	 * bInterrupted가 true이면 인터럽트에 의한 블렌드 아웃임을 의미합니다.
	 */
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted) const;

	/** 
	 * 능력이 취소될 때 호출되는 함수로, 몽타주 중단 및 관련 델리게이트 호출을 처리합니다.
	 */
	void OnAbilityCancelled() const;

	/** 
	 * 몽타주 재생이 종료되었을 때 호출되는 콜백 함수입니다.
	 * bInterrupted가 false이면 정상적으로 재생 완료된 경우로 처리합니다.
	 */
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/** 
	 * 지정된 게임플레이 이벤트가 발생했을 때 호출되는 콜백 함수입니다.
	 * 전달된 Payload를 통해 이벤트 데이터를 처리합니다.
	 */
	void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload) const;

	/** 
	 * 서버와의 동기화 후, 최종적으로 이벤트를 처리하는 함수입니다.
	 * 게임플레이 이벤트 델리게이트를 통해 이벤트가 전달됩니다.
	 */
	void OnServerSyncEventReceived(FGameplayTag EventTag, FGameplayEventData EventData) const;

private:
	/** 
	 * 현재 재생 중인 애니메이션 몽타주입니다.
	 * 이 변수는 재생 상태, 인터럽트 처리 등에서 참조됩니다.
	 */
	UPROPERTY()
	TObjectPtr<UAnimMontage> MontageToPlay;

	/** 
	 * 이벤트 발생 시 비교할 게임플레이 태그들을 보관하는 컨테이너입니다.
	 * 지정된 태그와 일치하는 이벤트가 발생하면, 해당 이벤트를 처리합니다.
	 */
	UPROPERTY()
	FGameplayTagContainer EventTags;

	/** 
	 * 몽타주 재생 속도를 결정하는 변수입니다. 기본값은 1.0f입니다.
	 * 이 값을 변경하면 애니메이션의 재생 속도가 조절됩니다.
	 */
	UPROPERTY()
	float Rate = 1.0f;

	/** 
	 * 몽타주 재생 시작 섹션의 이름을 저장합니다.
	 * 이를 통해 몽타주의 특정 섹션에서 재생을 시작할 수 있습니다.
	 */
	UPROPERTY()
	FName StartSection;

	/** 
	 * 루트 모션(root motion) 이동에 적용할 스케일 값입니다.
	 * 이 값에 따라 캐릭터의 실제 이동 거리가 조절됩니다.
	 */
	UPROPERTY()
	float AnimRootMotionTranslationScale;

	/** 
	 * 능력 종료 시 몽타주를 중단할지 여부를 결정하는 변수입니다.
	 * true이면 능력 종료 시 자동으로 몽타주 재생이 중단됩니다.
	 */
	UPROPERTY()
	bool bStopWhenAbilityEnds = true;


	// 델리게이트 및 핸들 변수들:
	// - BlendingOutDelegate: 몽타주 블렌드 아웃 상태 전환 시 사용
	// - MontageEndedDelegate: 몽타주 종료 시 사용
	// - CancelledHandle: 능력 취소 시 이벤트 핸들 관리
	// - EventHandle: 게임플레이 이벤트 바인딩 관리
	FOnMontageBlendingOutStarted BlendingOutDelegate;
	FOnMontageEnded MontageEndedDelegate;
	FDelegateHandle CancelledHandle;
	FDelegateHandle EventHandle;
};
