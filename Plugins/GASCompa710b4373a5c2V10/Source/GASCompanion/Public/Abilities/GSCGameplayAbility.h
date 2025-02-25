
#pragma once

#include "CoreMinimal.h"

#include "GSCTypes.h"
#include "Abilities/GameplayAbility.h"
#include "GSCGameplayAbility.generated.h"

/*
    이 파일은 GAS Companion 시스템과 함께 사용하기 위한 확장된 Gameplay Ability 클래스의 인터페이스를 정의합니다.
    이 클래스는 기본 UGameplayAbility에 추가적인 기능(예: GameplayEffect 컨테이너, 능력 큐, 비용 검사 완화, 자동 활성화 등)을 제공합니다.
*/

/**
 * @brief 능력 종료 시 호출되는 델리게이트.
 *
 * 이 델리게이트는 능력이 종료될 때(예: EndAbility 호출 시) 블루프린트나 다른 시스템에 알리기 위해 사용됩니다.
 * 주로 AI Behavior Tree 작업 등에서 능력 종료를 감지하여 후속 처리를 수행하는 데 유용합니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilityEnded);

/**
 * @brief GAS Companion용 확장 Gameplay Ability 클래스.
 *
 * 이 클래스는 기본 UGameplayAbility를 상속받아 GAS Companion 시스템에서 사용하기 위해 다음과 같은 추가 기능을 제공합니다
 * - GameplayEffect 컨테이너 지원: 효과를 그룹화하여 관리할 수 있으며, 효과 적용을 보다 체계적으로 할 수 있음.
 *   (자세한 내용은 https://github.com/tranek/GASDocumentation#concepts-ge-containers 참조)
 * - 능력 큐 시스템 지원: 능력 종료 후 다른 능력을 큐에 넣어 순차적으로 실행할 수 있도록 함.
 * - 비용 검사 완화(Loosely Check for Cost): 능력 비용이 음수로 떨어지더라도, 0 이하인 경우에만 실행을 제한하도록 함.
 * - 능력 종료 델리게이트: 능력 종료 시 블루프린트에서 할당 가능한 델리게이트를 노출하여, 종료 이벤트를 처리할 수 있음.
 * - 부여 시 자동 활성화(Activate On Granted): 수동으로 EndAbility를 호출하지 않아도, 부여된 즉시 자동으로 활성화되는 패시브 능력을 지원.
 *
 * 이 클래스는 GAS Companion과 통합되어, 게임 내에서 복잡한 능력 시스템을 효과적으로 관리할 수 있도록 설계되었습니다.
 */
UCLASS()
class GASCOMPANION_API UGSCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGSCGameplayAbility();

	/**
	 * @brief 효과 컨테이너로부터 효과 사양 컨테이너를 생성합니다.
	 *
	 * 주어진 FGSCGameplayEffectContainer와 FGameplayEventData를 기반으로,  나중에 효과를 적용할 수 있는 FGSCGameplayEffectContainerSpec을 생성합니다.
	 * 이 함수는 대상 선택(Targeting) 로직과 효과 사양 생성 로직을 포함합니다.
	 *
	 * @param Container 적용할 효과들의 정보를 담은 컨테이너.
	 * @param EventData 현재 발생한 게임플레이 이벤트 데이터.
	 * @param OverrideGameplayLevel (옵션) 재정의할 능력 레벨, INDEX_NONE인 경우 능력 자체의 레벨 사용.
	 * @return 생성된 FGSCGameplayEffectContainerSpec. 대상 및 효과 사양이 포함됩니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Ability", meta=(AutoCreateRefTerm = "EventData"))
	virtual FGSCGameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(const FGSCGameplayEffectContainer& Container,
	                                                                             const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

	/**
	 * @brief EffectContainerMap에서 지정된 태그에 해당하는 효과 컨테이너 사양을 생성합니다.
	 *
	 * 지정된 Gameplay Tag와 이벤트 데이터를 기반으로, 매핑된 효과 컨테이너를 찾아
	 * 나중에 적용할 효과 사양(FGSCGameplayEffectContainerSpec)을 생성합니다.
	 *
	 * @param ContainerTag 검색할 Gameplay Tag.
	 * @param EventData 현재 이벤트 데이터.
	 * @param OverrideGameplayLevel (옵션) 재정의할 게임플레이 레벨. 기본값은 -1.
	 * @return 생성된 FGSCGameplayEffectContainerSpec. 매핑된 컨테이너가 없으면 기본값을 반환.
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Ability", meta = (AutoCreateRefTerm = "EventData"))
	virtual FGSCGameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

	/**
	 * @brief 효과 컨테이너 사양을 적용하여 효과를 활성화합니다.
	 *
	 * 전달된 FGSCGameplayEffectContainerSpec의 각 효과 사양을 대상 데이터(ContainerSpec.TargetData)에 적용합니다.
	 * 적용된 각 효과의 핸들을 모두 모아 배열로 반환합니다.
	 *
	 * @param ContainerSpec 적용할 효과 사양 컨테이너.
	 * @return 활성화된 효과들의 FActiveGameplayEffectHandle 배열.
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Ability")
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(const FGSCGameplayEffectContainerSpec& ContainerSpec);

	/**
	 * @brief 효과 컨테이너를 생성하고 즉시 적용합니다.
	 * 지정된 Gameplay Tag와 이벤트 데이터를 기반으로 효과 컨테이너 사양을 생성한 후, 이를 적용하여 활성화된 효과 핸들의 배열을 반환합니다.
	 * @param ContainerTag 대상 Gameplay Tag.
	 * @param EventData 현재 이벤트 데이터.
	 * @param OverrideGameplayLevel (옵션) 재정의할 게임플레이 레벨. 기본값은 -1.
	 * @return 적용된 효과들의 FActiveGameplayEffectHandle 배열.
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Ability", meta = (AutoCreateRefTerm = "EventData"))
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainer(FGameplayTag ContainerTag,
	                                                                 const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

	//~Begin UGameplayAbility 인터페이스
public:
	/**
	 * @brief 아바타(Avatar)가 설정되었을 때 호출되는 함수.
	 *
	 * 능력의 아바타가 설정될 때, 특히 bActivateOnGranted가 true인 경우 능력을 자동으로 활성화합니다.
	 *
	 * @param ActorInfo 능력과 관련된 액터 정보.
	 * @param Spec 능력 사양 정보.
	 */
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	/**
	 * @brief 능력 비용 검사를 수행합니다.
	 *
	 * bLooselyCheckAbilityCost가 true이면, 비용 검사 없이 true를 반환하여 능력 활성화를 허용합니다.
	 * 그렇지 않은 경우, 기본 UGameplayAbility의 CheckCost 함수를 호출합니다.
	 *
	 * @param Handle 능력 사양 핸들.
	 * @param ActorInfo 능력 실행과 관련된 액터 정보.
	 * @param OptionalRelevantTags (옵션) 비용 관련 실패 태그를 추가할 컨테이너.
	 * @return 비용 지불이 가능하면 true, 아니면 false.
	 */
	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const override;

	/**
	 * @brief 능력을 활성화할 수 있는지 여부를 반환합니다.
	 *
	 * 이 함수는 능력 활성화 가능 여부를 판단하며, 비용 검사 완화 설정(bLooselyCheckAbilityCost)에 따라 비용 검사 로직을 유연하게 처리할 수 있습니다.
	 * Blueprints에서 별도로 CanActivateAbility를 구현할 경우 해당 함수가 호출되어 능력 활성 여부를 결정합니다.
	 *
	 * @param Handle 능력 사양 핸들.
	 * @param ActorInfo 능력 실행과 관련된 액터 정보.
	 * @param SourceTags (옵션) 소스 태그 컨테이너.
	 * @param TargetTags (옵션) 대상 태그 컨테이너.
	 * @param OptionalRelevantTags (옵션) 관련 태그 컨테이너. 필요 시 비용 실패 태그 등을 추가.
	 * @return 능력을 활성화할 수 있으면 true, 그렇지 않으면 false.
	 */
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	                                FGameplayTagContainer* OptionalRelevantTags) const override;
	//~End UGameplayAbility 인터페이스

	// Gameplay Abilities Delegates


	/**
	 * @brief 능력 종료 시 호출되는 함수.
	 * 내부적으로 OnAbilityEnded 델리게이트를 트리거하며, 능력이 종료될 때 추가적인 후처리를 수행할 수 있습니다.
	 * @param Ability 종료된 능력의 포인터.
	 */
	void AbilityEnded(UGameplayAbility* Ability);

protected:
	//~Begin UGameplayAbility 인터페이스

	/**
	 * @brief 능력 활성화 전 사전 처리 함수.
	 *
	 * 이 함수는 능력 활성화 전에 호출되어, 기본 사전 처리 작업을 수행한 후,
	 * 능력 종료 델리게이트를 등록하고, 능력 큐가 활성화되어 있으면 능력 큐를 엽니다.
	 *
	 * @param Handle 능력 사양 핸들.
	 * @param ActorInfo 능력 실행과 관련된 액터 정보.
	 * @param ActivationInfo 능력 활성화 정보.
	 * @param OnGameplayAbilityEndedDelegate 능력 종료 델리게이트.
	 * @param TriggerEventData (옵션) 트리거 이벤트 데이터.
	 */
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	                         const FGameplayEventData* TriggerEventData) override;

	//~End UGameplayAbility 인터페이스

private:
	/**
	 * @brief 비용 검사: 비용 속성이 양수인지 확인합니다.
	 *
	 * 능력 비용 효과(GetCostGameplayEffect)가 존재하면, 해당 효과의 속성 수정자들이 양수인지를 검사합니다.
	 * 만약 하나라도 속성 값이 0 이하이면 OptionalRelevantTags에 비용 실패 태그를 추가하고 false를 반환합니다.
	 *
	 * @param Handle 능력 사양 핸들.
	 * @param ActorInfo 능력 실행과 관련된 액터 정보.
	 * @param OptionalRelevantTags (OUT) 비용 실패 태그를 추가할 수 있는 컨테이너.
	 * @return 모든 비용 관련 속성이 양수이면 true, 아니면 false.
	 */
	bool CheckForPositiveCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const;

	/**
	 * @brief 속성 수정자들의 현재 값이 양수인지 검사합니다.
	 *
	 * 전달된 GameplayEffect를 기반으로 효과 사양을 생성하고, 계산된 수정자 magnitudes를 확인합니다.
	 * AbilitySystemComponent의 스폰된 속성들에서, 각 수정자가 적용될 속성의 현재 값을 확인합니다.
	 * 만약 Additive 연산자인 경우 속성 값이 0 이하이면 false를 반환합니다.
	 *
	 * @param GameplayEffect 검사할 GameplayEffect.
	 * @param ActorInfo 능력 실행과 관련된 액터 정보.
	 * @param Level 능력 레벨.
	 * @param EffectContext 효과 컨텍스트.
	 * @return 모든 Additive 속성 수정자의 현재 값이 양수이면 true, 하나라도 0 이하이면 false.
	 */
	bool CanApplyPositiveAttributeModifiers(const UGameplayEffect* GameplayEffect, const FGameplayAbilityActorInfo* ActorInfo, float Level, const FGameplayEffectContextHandle& EffectContext) const;

	/**
	 * @brief AbilitySystemComponent에서 특정 AttributeSet 인스턴스를 검색합니다.
	 *
	 * 이 함수는 AbilitySystemComponent에 스폰된 AttributeSet들 중, 지정된 AttributeClass의 인스턴스를 검색하여 반환합니다.
	 *
	 * @param AbilitySystemComponent 대상 AbilitySystemComponent.
	 * @param AttributeClass 검색할 UAttributeSet의 클래스.
	 * @return 찾은 UAttributeSet 포인터, 없으면 nullptr.
	 */
	static const UAttributeSet* GetAttributeSubobjectForASC(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UAttributeSet> AttributeClass);

public:
	/**
	 * @brief 능력 비용 검사 완화 플래그.
	 *
	 * 이 값이 true이면, 능력 비용이 음수로 떨어지는 것을 허용하여, 비용 검사 시 비용 속성의 현재 값이 0 이하인지 여부만 확인합니다.
	 * 즉, 비용 검사에서 엄격하게 음수를 허용하지 않으며, 오직 0 이하일 때만 활성화를 막습니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Ability")
	bool bLooselyCheckAbilityCost = false;

	/**
	 * @brief 부여 시 자동 활성화 플래그.
	 *
	 * 이 값이 true인 경우, 능력이 부여되자마자 자동으로 활성화됩니다.
	 * 패시브 능력이나 즉시 실행되어야 하는 일회성 능력을 구현할 때 유용합니다.
	 * 자동 활성화된 능력은 서버에서만 활성화되며, 일반적으로 Net Execution Policy가 Server Only로 설정됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Ability")
	bool bActivateOnGranted = false;

	/**
	 * @brief 능력 큐 활성화 플래그.
	 *
	 * 이 값이 true이면, 현재 능력이 종료된 후 다른 능력을 큐에 등록하고 실행할 수 있도록 합니다.
	 * 일반적으로는 false로 유지하고, montages 내 AbilityQueueNotifyState(AbilityQueueWindow)를 사용해 세밀하게 조정하는 것이 권장됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Ability")
	bool bEnableAbilityQueue = false;

	/**
	 * @brief Gameplay Tag와 GameplayEffect 컨테이너 매핑.
	 *
	 * 이 맵은 특정 Gameplay Tag가 부여되었을 때 적용할 GameplayEffect들을 그룹화하여 관리합니다.
	 * 각 태그에 대해 여러 효과 컨테이너를 설정할 수 있으며, 이를 통해 능력 실행 시 효과 적용을 체계적으로 관리할 수 있습니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS Companion|Ability")
	TMap<FGameplayTag, FGSCGameplayEffectContainer> EffectContainerMap;


	/**
	 * @brief 능력 종료 델리게이트.
	 *
	 * 이 델리게이트는 능력이 종료될 때 호출되며, 블루프린트에서 할당할 수 있습니다.
	 * 특히 AI Behavior Tree 등의 시스템에서 능력 종료를 감지하기 위해 사용됩니다.
	 */
	UPROPERTY(BlueprintAssignable, Category = "GAS Companion|Ability")
	FOnAbilityEnded OnAbilityEnded;
};
