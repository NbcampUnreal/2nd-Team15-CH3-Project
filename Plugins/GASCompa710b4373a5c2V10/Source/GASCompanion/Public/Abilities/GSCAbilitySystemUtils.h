#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"


class AActor;
class UAbilitySystemComponent;
class UActorComponent;
class UAttributeSet;
class UGSCAbilityInputBindingComponent;
class UGSCAbilitySet;
class UGameplayAbility;
class UGameplayEffect;
class UInputAction;

enum class EGSCAbilityTriggerEvent : uint8;
struct FActiveGameplayEffectHandle;
struct FComponentRequestHandle;
struct FGSCAbilitySetHandle;
struct FGSCGameFeatureAbilityMapping;
struct FGSCGameFeatureAttributeSetMapping;
struct FGameplayAbilitySpec;
struct FGameplayAbilitySpecHandle;
struct FGameplayTagContainer;

/**
 * @brief FGSCAbilitySystemUtils 클래스
 *
 * 이 클래스는 ASC(Ability System Component)에 다양한 능력, 속성, 효과, 태그 등을 부여하기 위한
 * 공통 유틸리티 함수를 static 멤버 함수 형태로 제공합니다.
 *
 * 주요 기능:
 * - 게임 기능(Game Feature) 액션 및 GSCAbilitySystemComponent 코드 경로에서 사용
 * - 능력(Ability), 속성(AttributeSet), 게임플레이 효과(Gameplay Effect) 부여
 * - 입력 바인딩 설정 및 중복 부여 방지 (이미 부여된 요소의 확인)
 * - Loose Gameplay Tags의 고유 추가/제거
 *
 * 이 유틸리티 클래스는 게임의 Ability System 관련 코드의 중복을 줄이고,
 * ASC에 대한 다양한 공통 작업을 손쉽게 수행할 수 있도록 설계되었습니다.
 */
class GASCOMPANION_API FGSCAbilitySystemUtils
{
public:
	/**
	 * @brief ASC에 능력을 부여하려 시도합니다.
	 *
	 * 주어진 FGSCGameFeatureAbilityMapping 정보를 기반으로, AbilitySystemComponent(InASC)에
	 * 능력을 부여하며, 부여된 능력의 핸들과 스펙 정보를 각각 OutAbilityHandle과 OutAbilitySpec에 반환합니다.
	 *
	 * @param InASC 능력을 부여할 대상 ASC(Ability System Component).
	 * @param InAbilityMapping 부여할 능력에 대한 매핑 정보 (능력 클래스, 레벨 등 포함).
	 * @param OutAbilityHandle 결과로 부여된 능력의 핸들 (능력 식별자 역할).
	 * @param OutAbilitySpec 결과로 부여된 능력의 스펙 (세부 설정 및 상태 정보).
	 */
	static void TryGrantAbility(UAbilitySystemComponent* InASC, const FGSCGameFeatureAbilityMapping& InAbilityMapping, FGameplayAbilitySpecHandle& OutAbilityHandle, FGameplayAbilitySpec& OutAbilitySpec);

	/**
	 * @brief ASC에 부여된 능력의 입력 바인딩을 설정합니다.
	 *
	 * 주어진 AbilityMapping과 능력의 스펙 정보를 이용해, ASC에 입력 액션과 트리거 이벤트를 연결합니다.
	 * 만약 능력이 이미 부여되어 있다면 해당 입력 바인딩을 바로 설정하고, 그렇지 않은 경우에는
	 * 능력이 부여된 후 입력 바인딩을 설정할 수 있도록 델리게이트를 등록합니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트.
	 * @param InAbilityMapping 능력에 대한 매핑 정보 (입력 액션, 트리거 이벤트 등 포함).
	 * @param InAbilityHandle 이미 부여된 능력의 핸들.
	 * @param InAbilitySpec 능력의 세부 스펙.
	 * @param OutOnGiveAbilityDelegateHandle 능력이 부여된 후 호출될 델리게이트 핸들.
	 * @param OutComponentRequests (선택적) 필요한 경우 추가 컴포넌트 요청 핸들을 저장할 배열.
	 */
	static void TryBindAbilityInput(
		UAbilitySystemComponent* InASC,
		const FGSCGameFeatureAbilityMapping& InAbilityMapping,
		const FGameplayAbilitySpecHandle& InAbilityHandle,
		const FGameplayAbilitySpec& InAbilitySpec,
		FDelegateHandle& OutOnGiveAbilityDelegateHandle,
		TArray<TSharedPtr<FComponentRequestHandle>>* OutComponentRequests = nullptr
	);

	/**
	 * @brief ASC에 속성(AttributeSet)을 부여하려 시도합니다.
	 *
	 * FGSCGameFeatureAttributeSetMapping에 따라 ASC에 속성을 추가하며, 이미 부여된 경우에는 기존 속성을 반환합니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트.
	 * @param InAttributeSetMapping 부여할 속성 클래스 및 초기화 데이터를 포함하는 매핑 정보.
	 * @param OutAttributeSet 부여된 속성의 UAttributeSet 포인터 (부여 실패 시 nullptr).
	 */
	static void TryGrantAttributes(UAbilitySystemComponent* InASC, const FGSCGameFeatureAttributeSetMapping& InAttributeSetMapping, UAttributeSet*& OutAttributeSet);

	/**
	 * @brief ASC에 게임플레이 효과(Gameplay Effect)를 적용하려 시도합니다.
	 *
	 * InEffectType으로 지정된 효과를 InLevel 레벨로 ASC에 적용하며, 이미 동일한 효과가 적용되어 있다면
	 * 해당 효과 핸들들을 OutEffectHandles에 반환합니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트.
	 * @param InEffectType 적용할 게임플레이 효과의 클래스.
	 * @param InLevel 효과의 적용 레벨.
	 * @param OutEffectHandles 적용된 효과 핸들들을 저장할 배열.
	 */
	static void TryGrantGameplayEffect(UAbilitySystemComponent* InASC, const TSubclassOf<UGameplayEffect> InEffectType, const float InLevel, TArray<FActiveGameplayEffectHandle>& OutEffectHandles);

	/**
	 * @brief ASC에 Ability Set을 부여하려 시도합니다.
	 *
	 * 주어진 Ability Set(InAbilitySet)에 포함된 능력, 속성, 효과, 태그 등을 ASC에 부여하고,
	 * 결과로 부여된 항목들의 정보를 OutAbilitySetHandle에 저장합니다.
	 * 필요에 따라 추가 컴포넌트 요청 핸들이 OutComponentRequests에 저장됩니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트.
	 * @param InAbilitySet 부여할 Ability Set (능력, 속성, 효과, 태그 등이 포함됨).
	 * @param OutAbilitySetHandle 부여된 Ability Set의 핸들 정보.
	 * @param OutComponentRequests (선택적) 추가 컴포넌트 요청 핸들을 저장할 배열.
	 * @return 부여 성공 시 true, 실패 시 false.
	 */
	static bool TryGrantAbilitySet(UAbilitySystemComponent* InASC, const UGSCAbilitySet* InAbilitySet, FGSCAbilitySetHandle& OutAbilitySetHandle, TArray<TSharedPtr<FComponentRequestHandle>>* OutComponentRequests = nullptr);

	/**
	 * @brief ASC에 부여된 속성 중 지정된 클래스의 속성을 반환합니다.
	 *
	 * ASC에 이미 부여되어 있는 UAttributeSet 중에서, InAttributeSet 클래스에 해당하는 속성을 찾습니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트 (const).
	 * @param InAttributeSet 찾고자 하는 속성의 클래스.
	 * @return 찾은 UAttributeSet 포인터; 없으면 nullptr 반환.
	 */
	static UAttributeSet* GetAttributeSet(const UAbilitySystemComponent* InASC, const TSubclassOf<UAttributeSet> InAttributeSet);

	/**
	 * @brief ASC에 특정 게임플레이 효과가 이미 적용되었는지 확인합니다.
	 *
	 * 동일한 효과 클래스(InEffectType) 및 레벨의 효과가 ASC에 적용되어 있다면, 해당 효과 핸들을
	 * OutEffectHandles에 저장하고 true를 반환합니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트 (const).
	 * @param InEffectType 확인할 게임플레이 효과의 클래스.
	 * @param OutEffectHandles 이미 적용된 효과 핸들들을 저장할 배열.
	 * @return 효과가 이미 적용되었다면 true, 아니면 false.
	 */
	static bool HasGameplayEffectApplied(const UAbilitySystemComponent* InASC, const TSubclassOf<UGameplayEffect>& InEffectType, TArray<FActiveGameplayEffectHandle>& OutEffectHandles);

	/**
	 * @brief ASC에 특정 능력이 이미 부여되었는지 확인합니다.
	 *
	 * 능력 클래스(InAbility)와 레벨(InLevel)을 기준으로, ASC에 이미 부여된 능력이 존재하는지 검사합니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트 (const).
	 * @param InAbility 확인할 능력의 클래스.
	 * @param InLevel 확인할 능력의 레벨 (기본값: 1).
	 * @return 능력이 이미 부여되어 있으면 true, 아니면 false.
	 */
	static bool IsAbilityGranted(const UAbilitySystemComponent* InASC, TSubclassOf<UGameplayAbility> InAbility, const int32 InLevel = 1);

	// 아래 템플릿 함수는 C++20 확장 경고로 인해 주석 처리되었습니다.
	/*
	// template<class ComponentType>
	// static ComponentType* FindOrAddComponentForActor(AActor* InActor, TArray<TSharedPtr<FComponentRequestHandle>>& OutComponentRequests)
	// {
	// 	return Cast<ComponentType>(FindOrAddComponentForActor(ComponentType::StaticClass(), InActor, OutComponentRequests));
	// }
	*/

	/**
	 * @brief 지정된 액터에서 원하는 컴포넌트를 찾거나, 존재하지 않으면 추가합니다.
	 *
	 * InActor에서 InComponentType 타입의 컴포넌트를 검색하며, 없을 경우 GameFrameworkComponentManager를 통해
	 * 컴포넌트 요청을 수행하여 해당 컴포넌트를 추가합니다.
	 *
	 * @param InComponentType 찾거나 추가할 컴포넌트의 클래스.
	 * @param InActor 컴포넌트를 검색할 대상 액터.
	 * @param OutComponentRequests 컴포넌트 요청 핸들을 저장할 배열 (추가 요청 발생 시).
	 * @return 찾거나 추가된 UActorComponent 포인터.
	 */
	static UActorComponent* FindOrAddComponentForActor(UClass* InComponentType, const AActor* InActor, TArray<TSharedPtr<FComponentRequestHandle>>& OutComponentRequests);

	/**
	 * @brief ASC에 Loose Gameplay Tags를 고유하게 추가합니다.
	 *
	 * ASC가 이미 보유하지 않은 태그들만 InTags에서 추려내어 추가하며,
	 * bReplicated가 true이면 복제용 태그도 함께 추가합니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트.
	 * @param InTags 추가할 태그들을 포함한 컨테이너.
	 * @param bReplicated 복제 여부 (기본값: true).
	 */
	static void AddLooseGameplayTagsUnique(UAbilitySystemComponent* InASC, const FGameplayTagContainer& InTags, const bool bReplicated = true);

	/**
	 * @brief ASC에서 Loose Gameplay Tags를 고유하게 제거합니다.
	 *
	 * ASC가 보유하고 있는 태그들 중 InTags에 포함된 태그들을 찾아 제거하며,
	 * bReplicated가 true이면 복제용 태그도 함께 제거합니다.
	 *
	 * @param InASC 능력 시스템 컴포넌트.
	 * @param InTags 제거할 태그들을 포함한 컨테이너.
	 * @param bReplicated 복제 여부 (기본값: true).
	 */
	static void RemoveLooseGameplayTagsUnique(UAbilitySystemComponent* InASC, const FGameplayTagContainer& InTags, const bool bReplicated = true);

private:
	/**
	 * @brief AbilitySystem의 OnGiveAbility 델리게이트 핸들러.
	 *
	 * 능력이 ASC에 부여될 때 클라이언트 측에서 호출되어, 입력 바인딩 컴포넌트와 입력 액션, 트리거 이벤트를 기반으로
	 * 입력 바인딩을 설정합니다. 이 함수는 능력 부여 후, 클라이언트가 올바른 입력 바인딩을 구성할 수 있도록 도와줍니다.
	 *
	 * @param InAbilitySpec 부여된 능력의 스펙 (세부 정보).
	 * @param InInputComponent 약한 포인터 형태의 UGSCAbilityInputBindingComponent. 입력 바인딩을 설정할 대상.
	 * @param InInputAction 약한 포인터 형태의 UInputAction. 연결할 입력 액션.
	 * @param InTriggerEvent 입력 트리거 이벤트 (예: 버튼 눌림, 해제 등).
	 * @param InNewAbilitySpec 새로 부여된 능력의 스펙 (업데이트된 정보).
	 */
	static void HandleOnGiveAbility(
		FGameplayAbilitySpec& InAbilitySpec,
		TWeakObjectPtr<UGSCAbilityInputBindingComponent> InInputComponent,
		TWeakObjectPtr<UInputAction> InInputAction,
		const EGSCAbilityTriggerEvent InTriggerEvent,
		FGameplayAbilitySpec InNewAbilitySpec
	);
};
