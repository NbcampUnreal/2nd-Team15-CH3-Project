#pragma once

#include "CoreMinimal.h"
#include "Abilities/GSCGameplayAbility.h"
#include "GSCGameplayAbility_MeleeBase.generated.h"

/*
===========================================================================================
  파일 개요:
    UGSCGameplayAbility_MeleeBase 클래스는 근접 공격 관련 능력의 기본 구현을 제공합니다.
    이 클래스는 UGSCGameplayAbility를 상속받아, 근접 공격 시 사용할 애니메이션 몽타주, 
    콤보 매니저 컴포넌트, 그리고 이벤트 기반 효과 적용 등 근접 공격 능력의 핵심 기능들을 포함합니다.
===========================================================================================
*/

/*------------------------------------------------------------------------------
  전방 선언
------------------------------------------------------------------------------*/
// 콤보 관련 로직을 관리하는 컴포넌트 클래스
class UGSCComboManagerComponent;

/*------------------------------------------------------------------------------
  UGSCGameplayAbility_MeleeBase 클래스 선언
------------------------------------------------------------------------------*/

/**
 * @brief 근접 공격 기본 능력 클래스.
 *
 * 이 클래스는 근접 공격 능력의 기본 동작을 구현합니다.
 * - 콤보 매니저를 통해 콤보 수를 관리하며,  여러 애니메이션 몽타주를 순차적으로 재생하여 다양한 공격 콤보를 구성합니다.
 * - 지정된 이벤트 태그가 발생하면 효과를 적용합니다.
 *
 * 이 클래스는 GAS Companion 시스템의 근접 공격 능력 구현에 사용됩니다.
 */
UCLASS()
class GASCOMPANION_API UGSCGameplayAbility_MeleeBase : public UGSCGameplayAbility
{
	GENERATED_BODY()

	// ==========================================================================
	//  Public Interface
	//    - 생성자 및 공개 멤버 함수
	// ==========================================================================
public:
	/**
	 * @brief 기본 생성자.
	 *
	 * UGSCGameplayAbility_MeleeBase 클래스의 인스턴스를 생성하며, 
	 * 필요한 초기화를 수행합니다.
	 */
	UGSCGameplayAbility_MeleeBase();

	//-------------------------------------
	// 콤보 및 애니메이션 관련 공개 함수
	//-------------------------------------
	/**
	 * @brief 다음 콤보 애니메이션 몽타주를 반환합니다.
	 *
	 * 콤보 매니저 컴포넌트에 의해 관리되는 현재 콤보 인덱스를 기반으로, 재생할 다음 애니메이션 몽타주를 선택합니다.
	 *
	 * @return 다음에 재생할 UAnimMontage 포인터, 유효하지 않으면 nullptr.
	 */
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Ability|Melee")
	UAnimMontage* GetNextComboMontage();


	// ==========================================================================
	//  Protected Interface
	//    - 내부 구현에 사용되는 멤버 함수 및 변수
	// ==========================================================================
protected:
	//-------------------------------------
	// 능력 활성화 관련 오버라이드 함수
	//-------------------------------------
	
	/**
	 * @brief 능력 활성화 처리 함수.
	 *
	 * 이 함수는 근접 공격 능력이 활성화될 때 호출되며, 
	 * 콤보 매니저를 통한 콤보 증가, 다음 몽타주 재생, 
	 * 그리고 몽타주 관련 태스크(UGSCTask_PlayMontageWaitForEvent)를 생성하여 실행합니다.
	 *
	 * @param Handle 능력 사양 핸들.
	 * @param ActorInfo 능력 실행과 관련된 액터 정보.
	 * @param ActivationInfo 능력 활성화 정보.
	 * @param TriggerEventData (옵션) 트리거 이벤트 데이터.
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                             const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	//-------------------------------------
	// 몽타주 및 이벤트 콜백 함수
	//-------------------------------------
	UFUNCTION()
	void OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

	/**
	 * @brief 몽타주 완료 시 호출되는 콜백 함수.
	 *
	 * 몽타주가 정상적으로 완료되면 호출되어, 능력을 종료합니다.
	 *
	 * @param EventTag 발생한 이벤트 태그.
	 * @param EventData 관련 이벤트 데이터.
	 */
	UFUNCTION()
	void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	/**
	 * @brief 지정된 게임플레이 이벤트 수신 시 호출되는 콜백 함수.
	 *
	 * 이 함수는 몽타주 재생 중 지정된 이벤트가 발생하면 호출되어,
	 * 해당 이벤트에 대응하는 효과 컨테이너를 적용합니다.
	 *
	 * @param EventTag 발생한 이벤트 태그.
	 * @param EventData 관련 이벤트 데이터.
	 */
	UFUNCTION()
	void OnEventReceived(FGameplayTag EventTag, FGameplayEventData EventData);

protected:
	//-------------------------------------
	// 내부 컴포넌트 및 설정 변수
	//-------------------------------------
	/**
	 * @brief 콤보 매니저 컴포넌트.
	 *
	 * 이 컴포넌트는 근접 공격 콤보의 횟수를 관리하며, 공격 콤보 진행 상태를 저장합니다.
	 * 능력 활성화 시, 아바타 액터에서 해당 컴포넌트를 검색하여 할당됩니다.
	 */
	UPROPERTY()
	TObjectPtr<UGSCComboManagerComponent> ComboManagerComponent;

	/**
	 * @brief 재생할 애니메이션 몽타주 목록.
	 *
	 * 이 배열에는 근접 공격 시 순차적으로 재생할 애니메이션 몽타주들이 저장됩니다.
	 * 각 몽타주는 공격 콤보의 다양한 단계를 표현합니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Montages")
	TArray<TObjectPtr<UAnimMontage>> Montages;

	/**
	 * @brief 애니메이션 재생 속도.
	 *
	 * 이 값은 몽타주 재생 속도를 조절하는데 사용되며, 기본값은 1.0f입니다.
	 * 값이 클수록 애니메이션이 빨리 재생됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Montages")
	float Rate = 1.f;

	/**
	 * @brief 이벤트 태그 컨테이너.
	 *
	 * 이 태그 컨테이너에 포함된 태그와 일치하는 게임플레이 이벤트가 발생하면, OnEventReceived 콜백이 호출되어 능력에 정의된 효과 컨테이너를 적용합니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Montages")
	FGameplayTagContainer WaitForEventTag;
};
