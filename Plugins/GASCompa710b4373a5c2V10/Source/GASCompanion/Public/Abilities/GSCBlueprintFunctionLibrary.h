#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Abilities/GSCAbilitySet.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GSCBlueprintFunctionLibrary.generated.h"

/*
    전방 선언:
    아래에 선언된 클래스들은 이 블루프린트 함수 라이브러리에서 사용되는 컴포넌트 및 시스템 인터페이스를 나타냅니다.
    이들 클래스는 게임 내에서 Ability System, 입력 바인딩, 콤보 시스템, 코어 기능 등을 담당합니다.
*/
class UGSCAbilityInputBindingComponent;
class UGSCAbilityQueueComponent;
class UGSCAbilitySystemComponent;
class UGSCComboManagerComponent;
class UGSCCoreComponent;

/**
 * @brief Ability 관련 블루프린트 함수 라이브러리
 *
 * 이 클래스는 게임의 능력(Ability) 시스템과 관련된 기능들을 블루프린트에서 사용할 수 있도록
 * 노출하는 함수들을 제공하는 라이브러리입니다.
*
 * 주요 역할:
 * - Ability System Component, 콤보 매니저, 코어 컴포넌트, 입력 바인딩 컴포넌트 등을 액터로부터 검색하여 반환합니다.
 * - Loose Gameplay Tags(게임플레이 효과에 의해 부여되지 않은 태그)를 액터의 AbilitySystemComponent에 추가 및 제거합니다.
 * - Ability System Component에 대한 다양한 Gameplay Cue(게임플레이 큐) 호출을 지원합니다.
 * - Ability Set에 관련된 디버그 정보를 반환하는 기능을 제공합니다.
 *
 * 이 라이브러리를 사용하면, 게임 내에서 능력 관련 컴포넌트들에 쉽게 접근하고 조작할 수 있으며, 코드와 블루프린트 간의 인터페이스 역할을 수행하여 개발의 효율성을 높입니다.
*/
UCLASS()
class GASCOMPANION_API UGSCBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * @brief 액터로부터 UGSCAbilitySystemComponent를 찾습니다.
	 *
	 * 주어진 액터에 연결된 AbilitySystemComponent를 검색하고, 해당 컴포넌트가 UGSCAbilitySystemComponent 타입으로
	 * 캐스팅 가능한 경우 반환합니다. AbilitySystemInterface를 사용하거나, 컴포넌트 검색을 통해 찾습니다.
	 *
	 * @param Actor 대상 액터 (Ability System Component가 부착되어 있어야 함)
	 * @return UGSCAbilitySystemComponent 포인터. 찾지 못하면 nullptr 반환.
	 */
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Components")
	static UGSCAbilitySystemComponent* GetCompanionAbilitySystemComponent(const AActor* Actor);

	/**
	 * @brief 액터로부터 콤보 매니저 컴포넌트를 찾습니다.
	 *
	 * 이 함수는 액터에 부착된 UGSCComboManagerComponent를 검색하여 반환합니다.
	 * 콤보 매니저는 연속 공격 등의 콤보 로직을 관리하는 데 사용됩니다.
	 *
	 * @param Actor 대상 액터
	 * @return UGSCComboManagerComponent 포인터. 컴포넌트가 없으면 nullptr 반환.
	*/
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Components")
	static UGSCComboManagerComponent* GetComboManagerComponent(const AActor* Actor);

	/**
	 * @brief 액터로부터 Companion Core 컴포넌트를 찾습니다.
	 *
	 * Companion Core 컴포넌트는 게임의 핵심 로직 및 데이터 처리를 담당하는 컴포넌트입니다.
	 * 이 함수는 액터에서 UGSCCoreComponent를 검색하여 반환합니다.
	 *
	 * @param Actor 대상 액터
	 * @return UGSCCoreComponent 포인터. 없으면 nullptr 반환.
	*/
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Components")
	static UGSCCoreComponent* GetCompanionCoreComponent(const AActor* Actor);

	/**
	 * @brief 액터로부터 Ability Queue 컴포넌트를 찾습니다.
	 *
	 * Ability Queue 컴포넌트는 능력 발동 순서를 관리하는 기능을 제공합니다.
	 * 이 함수는 액터에서 UGSCAbilityQueueComponent를 검색하여 반환합니다.
	 *
	 * @param Actor 대상 액터
	 * @return UGSCAbilityQueueComponent 포인터. 컴포넌트가 없으면 nullptr 반환.
	*/
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Components")
	static UGSCAbilityQueueComponent* GetAbilityQueueComponent(const AActor* Actor);

	/**
	 * @brief 액터로부터 Ability Input Binding 컴포넌트를 찾습니다.
	 *
	 * 이 함수는 액터에 부착된 UGSCAbilityInputBindingComponent를 검색하여 반환합니다.
	 * 입력 바인딩 컴포넌트는 능력 시스템에서 입력 이벤트와 능력 실행을 연결하는 역할을 합니다.
	 *
	 * @param Actor 대상 액터
	 * @return UGSCAbilityInputBindingComponent 포인터. 없으면 nullptr 반환.
	*/
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Components")
	static UGSCAbilityInputBindingComponent* GetAbilityInputBindingComponent(const AActor* Actor);

	/** Gameplay Tags 관련 함수 */

	/**
	 * @brief 액터의 AbilitySystemComponent에 Loose Gameplay Tags를 추가합니다.
	*
	 * AbilitySystemInterface를 사용하여 액터로부터 AbilitySystemComponent를 찾고,
	 * 해당 컴포넌트에 게임플레이 효과에 의해 부여되지 않은 Loose Gameplay Tags를 추가합니다.
	 * 이 태그들은 복제되지 않으며, 클라이언트/서버 별로 별도 관리해야 합니다.
	*
	 * @param Actor 대상 액터
	 * @param GameplayTags 추가할 Gameplay Tag들이 담긴 컨테이너
	 * @return 태그 추가에 성공하면 true, 실패하면 false 반환.
	*/
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities|GameplayTags")
	static bool AddLooseGameplayTagsToActor(AActor* Actor, const FGameplayTagContainer GameplayTags);

	/**
	 * @brief 액터의 AbilitySystemComponent에서 Loose Gameplay Tags를 제거합니다.
	*
	 * AbilitySystemInterface를 사용하여 액터로부터 AbilitySystemComponent를 찾고,
	 * 해당 컴포넌트에서 게임플레이 효과에 의해 부여되지 않은 Loose Gameplay Tags를 제거합니다.
	 * 이 태그들은 복제되지 않으므로, 클라이언트/서버 별로 별도 관리가 필요합니다.
	*
	 * @param Actor 대상 액터
	 * @param GameplayTags 제거할 Gameplay Tag들이 담긴 컨테이너
	 * @return 태그 제거에 성공하면 true, 실패하면 false 반환.
	*/
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities|GameplayTags")
	static bool RemoveLooseGameplayTagsFromActor(AActor* Actor, const FGameplayTagContainer GameplayTags);

	/**
	 * @brief 액터의 AbilitySystemComponent가 지정된 Gameplay Tag와 일치하는 태그를 가지고 있는지 확인합니다.
	 *
	 * 이 함수는 액터의 AbilitySystemComponent에서 지정된 Gameplay Tag(및 부모 태그 포함)를 검색하여,
	 * 일치하는 태그가 존재하면 true를 반환합니다.
	 *
	 * @param Actor 대상 액터
	 * @param GameplayTag 확인할 Gameplay Tag
	 * @return 일치하는 태그가 있으면 true, 없으면 false 반환.
	*/
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Abilities|GameplayTags")
	static bool HasMatchingGameplayTag(AActor* Actor, const FGameplayTag GameplayTag);

	/**
	 * @brief 액터의 AbilitySystemComponent가 지정된 Gameplay Tag 컨테이너 중 하나라도 포함하는지 확인합니다.
	 *
	 * 이 함수는 액터의 AbilitySystemComponent에서 주어진 태그 컨테이너에 포함된 태그(및 부모 태그 포함)를 검색하여,
	 * 하나라도 일치하면 true를 반환합니다.
	 *
	 * @param Actor 대상 액터
	 * @param GameplayTags 확인할 Gameplay Tag들이 담긴 컨테이너
	 * @return 하나 이상의 태그가 일치하면 true, 아니면 false 반환.
	*/
	UFUNCTION(BlueprintPure, Category = "GAS Companion|Abilities|GameplayTags")
	static bool HasAnyMatchingGameplayTag(AActor* Actor, const FGameplayTagContainer GameplayTags);

	/**
	 * @brief 지정된 Gameplay Attribute의 이름을 문자열로 반환합니다.
	 *
	 * 이 함수는 FGameplayAttribute 구조체에 포함된 속성의 이름을 얻어 디버깅 및 표시 목적으로 사용됩니다.
	 *
	 * @param Attribute 확인할 Gameplay Attribute
	 * @return 속성의 이름을 나타내는 FString.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GAS Companion|Attribute Set")
	static FString GetDebugStringFromAttribute(FGameplayAttribute Attribute);

	/**
	 * @brief 제공된 AttributeSet 클래스에 정의된 모든 Gameplay Attribute를 반환합니다.
	 *
	 * 이 함수는 특정 UAttributeSet 클래스의 모든 속성들을 순회하여, 각 속성을 FGameplayAttribute 형태로
	 * OutAttributes 배열에 추가합니다. 이를 통해 해당 AttributeSet에서 사용 가능한 모든 속성을 확인할 수 있습니다.
	 *
	 * @param AttributeSetClass 대상 AttributeSet 클래스 (UAttributeSet의 서브클래스)
	 * @param OutAttributes 반환될 Gameplay Attribute들의 배열
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GAS Companion|Attribute Set")
	static void GetAllAttributes(TSubclassOf<UAttributeSet> AttributeSetClass, TArray<FGameplayAttribute>& OutAttributes);

	/**
	 * @brief Gameplay Attribute와 문자열이 서로 다름을 확인합니다.
	 *
	 * 이 함수는 FGameplayAttribute의 이름과 주어진 문자열을 비교하여, 두 값이 일치하지 않으면 true를 반환합니다.
	 * 주로 핀 옵션 및 디버깅에서 사용됩니다.
	 *
	 * @param A 비교할 Gameplay Attribute
	 * @param B 비교할 문자열
	 * @return 두 값이 다르면 true, 같으면 false.
	 */
	UFUNCTION(BlueprintPure, Category = "GAS Companion|PinOptions", meta = (BlueprintInternalUseOnly = "TRUE"))
	static bool NotEqual_GameplayAttributeGameplayAttribute(FGameplayAttribute A, FString B);

	// -------------------------------------
	//	GameplayCue 관련 함수
	//	AbilitySystemComponent를 통해 GameplayCue를 실행하거나 추가/제거할 수 있습니다.
	// -------------------------------------

	/**
	 * @brief 액터의 AbilitySystemComponent에서 GameplayCue를 실행합니다.
	 *
	 * 주어진 GameplayCue Tag와 Effect Context를 사용하여, 액터의 AbilitySystemComponent에 설정된
	 * GameplayCue를 실행합니다. 이 함수는 즉시 실행되는 효과를 발생시킵니다.
	 *
	 * @param Actor 대상 액터
	 * @param GameplayCueTag 실행할 GameplayCue의 태그
	 * @param Context Gameplay Effect Context (발생 조건 및 소스 정보 포함)
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities|GameplayCue", meta=(GameplayTagFilter="GameplayCue"))
	static void ExecuteGameplayCueForActor(AActor* Actor, FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context);

	/**
	 * @brief 추가 매개변수를 포함하여 액터의 AbilitySystemComponent에서 GameplayCue를 실행합니다.
	 *
	 * 주어진 GameplayCue Tag와 GameplayCueParameters를 사용하여, 더 세부적인 설정과 함께 GameplayCue를 실행합니다.
	 *
	 * @param Actor 대상 액터
	 * @param GameplayCueTag 실행할 GameplayCue의 태그
	 * @param GameplayCueParameters 실행 시 적용할 추가 파라미터들
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities|GameplayCue", meta=(GameplayTagFilter="GameplayCue"))
	static void ExecuteGameplayCueWithParams(AActor* Actor, FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters);

	/**
	 * @brief 액터의 AbilitySystemComponent에 지속형 GameplayCue를 추가합니다.
	 *
	 * 지정된 GameplayCue Tag와 Effect Context를 사용하여, 액터의 AbilitySystemComponent에 지속적으로
	 * 활성화된 GameplayCue를 추가합니다. 이 큐는 필요에 따라 능력 종료 시 제거될 수 있습니다.
	 *
	 * @param Actor 대상 액터
	 * @param GameplayCueTag 추가할 GameplayCue의 태그
	 * @param Context Gameplay Effect Context
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities|GameplayCue", meta=(GameplayTagFilter="GameplayCue"))
	static void AddGameplayCue(AActor* Actor, FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context);

	/**
	 * @brief 추가 매개변수를 포함하여 액터의 AbilitySystemComponent에 지속형 GameplayCue를 추가합니다.
	 *
	 * 지정된 GameplayCue Tag와 GameplayCueParameters를 사용하여, 더욱 세부적인 설정과 함께 지속형 GameplayCue를 추가합니다.
	 *
	 * @param Actor 대상 액터
	 * @param GameplayCueTag 추가할 GameplayCue의 태그
	 * @param GameplayCueParameter 추가 매개변수를 포함한 GameplayCueParameters
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities|GameplayCue", meta=(GameplayTagFilter="GameplayCue"))
	static void AddGameplayCueWithParams(AActor* Actor, FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameter);

	/**
	 * @brief 액터의 AbilitySystemComponent에서 특정 GameplayCue를 제거합니다.
	 *
	 * 지정된 GameplayCue Tag에 해당하는 큐를 액터의 AbilitySystemComponent에서 제거합니다.
	 *
	 * @param Actor 대상 액터
	 * @param GameplayCueTag 제거할 GameplayCue의 태그
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities|GameplayCue", meta=(GameplayTagFilter="GameplayCue"))
	static void RemoveGameplayCue(AActor* Actor, FGameplayTag GameplayCueTag);

	/**
	 * @brief 액터의 AbilitySystemComponent에서 모든 GameplayCue를 제거합니다.
	 *
	 * 이 함수는 액터의 AbilitySystemComponent에 의해 개별적으로 추가된(게임플레이 효과에 의존하지 않는)
	 * 모든 GameplayCue를 제거합니다.
	 *
	 * @param Actor 대상 액터
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities|GameplayCue", meta=(GameplayTagFilter="GameplayCue"))
	static void RemoveAllGameplayCues(AActor* Actor);

	// -------------------------------------
	//	Ability Set 관련 함수
	// -------------------------------------

	/**
	 * @brief Ability Set 핸들의 문자열 표현을 반환합니다.
	 *
	 * 주어진 FGSCAbilitySetHandle에 저장된 정보를 기반으로, Ability Set의 상태나 구성을 나타내는
	 * 디버그 문자열을 반환합니다.
	 *
	 * @param InAbilitySetHandle Ability Set 핸들 (능력, 속성, 효과, 태그 정보 포함)
	 * @param bVerbose 상세 정보 출력 여부 (true이면 상세 정보를 포함한 문자열 반환)
	 * @return Ability Set의 상태를 나타내는 FString.
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Ability Sets")
	static FString DebugAbilitySetHandle(const FGSCAbilitySetHandle& InAbilitySetHandle, const bool bVerbose = false);
};
