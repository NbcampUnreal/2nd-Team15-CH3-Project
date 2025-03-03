// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Abilities/GSCBlueprintFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemGlobals.h"
#include "GSCLog.h"
#include "Abilities/GSCAbilitySystemComponent.h"
#include "Components/GSCAbilityInputBindingComponent.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCComboManagerComponent.h"
#include "Components/GSCCoreComponent.h"

/**
 * @brief 액터에서 UGSCAbilitySystemComponent를 검색하여 반환합니다.
 *
 * 주어진 액터에 부착된 AbilitySystemComponent를 AbilitySystemGlobals::GetAbilitySystemComponentFromActor 함수를 통해 검색합니다.
 * 만약 검색된 ASC가 UGSCAbilitySystemComponent 타입으로 캐스팅 가능하면 해당 포인터를 반환하며,
 * 그렇지 않을 경우 경고 로그를 출력하고 nullptr를 반환합니다.
 *
 * @param Actor 검색할 대상 액터. (nullptr이면 검색 실패)
 * @return UGSCAbilitySystemComponent 포인터. 성공하면 유효한 포인터, 실패하면 nullptr.
 */
UGSCAbilitySystemComponent* UGSCBlueprintFunctionLibrary::GetCompanionAbilitySystemComponent(const AActor* Actor)
{
	// AbilitySystemComponent를 액터로부터 검색합니다.
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!ASC)
	{
		return nullptr;
	}

	// 검색된 ASC가 UGSCAbilitySystemComponent 타입인지 확인합니다.
	if (UGSCAbilitySystemComponent* CompanionASC = Cast<UGSCAbilitySystemComponent>(ASC))
	{
		return CompanionASC;
	}

	// 타입이 일치하지 않을 경우 경고 로그 출력 (영어 메시지를 한글로 변경)
	GSC_PLOG(Warning, TEXT("ASC `%s` 는 액터 `%s`의 소유자가 UGSCAbilitySystemComponent 타입이 아닙니다."), *GetNameSafe(ASC), *GetNameSafe(Actor));
	return nullptr;
}

/**
 * @brief 액터에서 UGSCComboManagerComponent를 검색하여 반환합니다.
 *
 * 액터에 부착된 UGSCComboManagerComponent를 FindComponentByClass 템플릿 함수를 사용하여 검색합니다.
 *
 * @param Actor 검색할 대상 액터.
 * @return UGSCComboManagerComponent 포인터. 컴포넌트가 없으면 nullptr.
 */
UGSCComboManagerComponent* UGSCBlueprintFunctionLibrary::GetComboManagerComponent(const AActor* Actor)
{
	// 액터의 유효성 검사를 수행합니다.
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// 컴포넌트를 검색하여 반환합니다.
	return Actor->FindComponentByClass<UGSCComboManagerComponent>();
}

/**
 * @brief 액터에서 UGSCCoreComponent를 검색하여 반환합니다.
 *
 * 액터에 부착된 코어 컴포넌트를 FindComponentByClass 함수를 사용해 검색합니다.
 *
 * @param Actor 검색할 대상 액터.
 * @return UGSCCoreComponent 포인터. 없으면 nullptr.
 */
UGSCCoreComponent* UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(const AActor* Actor)
{
	// 액터의 유효성을 검사합니다.
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// 코어 컴포넌트를 검색하여 반환합니다.
	return Actor->FindComponentByClass<UGSCCoreComponent>();
}

/**
 * @brief 액터에서 UGSCAbilityQueueComponent를 검색하여 반환합니다.
 *
 * 액터에 부착된 Ability Queue 컴포넌트를 검색합니다.
 *
 * @param Actor 검색할 대상 액터.
 * @return UGSCAbilityQueueComponent 포인터. 컴포넌트가 없으면 nullptr.
 */
UGSCAbilityQueueComponent* UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(const AActor* Actor)
{
	// 액터의 유효성을 검사합니다.
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// Ability Queue 컴포넌트를 검색하여 반환합니다.
	return Actor->FindComponentByClass<UGSCAbilityQueueComponent>();
}

/**
 * @brief 액터에서 UGSCAbilityInputBindingComponent를 검색하여 반환합니다.
 *
 * 액터에 부착된 Ability Input Binding 컴포넌트를 검색하여 반환합니다.
 *
 * @param Actor 검색할 대상 액터.
 * @return UGSCAbilityInputBindingComponent 포인터. 없으면 nullptr.
 */
UGSCAbilityInputBindingComponent* UGSCBlueprintFunctionLibrary::GetAbilityInputBindingComponent(const AActor* Actor)
{
	// 액터의 유효성을 검사합니다.
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	// 컴포넌트를 검색하여 반환합니다.
	return Actor->FindComponentByClass<UGSCAbilityInputBindingComponent>();
}

/**
 * @brief 액터에 Loose Gameplay Tags를 추가합니다.
 *
 * 주어진 액터에서 AbilitySystemComponent를 검색한 후, 해당 컴포넌트에 인자로 전달된 GameplayTags를 추가합니다.
 * 태그 추가는 AbilitySystemBlueprintLibrary를 통해 수행됩니다.
 *
 * @param Actor 태그를 추가할 대상 액터.
 * @param GameplayTags 추가할 Gameplay Tag들이 담긴 컨테이너.
 * @return 태그 추가가 성공하면 true, 실패하면 false.
 */
bool UGSCBlueprintFunctionLibrary::AddLooseGameplayTagsToActor(AActor* Actor, const FGameplayTagContainer GameplayTags)
{
	// 액터로부터 AbilitySystemComponent를 검색합니다.
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);

	if (AbilitySystemComponent)
	{
		// Loose Gameplay Tags를 추가합니다.
		AbilitySystemComponent->AddLooseGameplayTags(GameplayTags);
		return true;
	}

	return false;
}

/**
 * @brief 액터에서 Loose Gameplay Tags를 제거합니다.
 *
 * 주어진 액터의 AbilitySystemComponent를 검색하여, 인자로 전달된 GameplayTags를 제거합니다.
 *
 * @param Actor 태그를 제거할 대상 액터.
 * @param GameplayTags 제거할 Gameplay Tag들이 담긴 컨테이너.
 * @return 태그 제거가 성공하면 true, 실패하면 false.
 */
bool UGSCBlueprintFunctionLibrary::RemoveLooseGameplayTagsFromActor(AActor* Actor, const FGameplayTagContainer GameplayTags)
{
	// 액터로부터 AbilitySystemComponent를 검색합니다.
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor);

	if (AbilitySystemComponent)
	{
		// Loose Gameplay Tags를 제거합니다.
		AbilitySystemComponent->RemoveLooseGameplayTags(GameplayTags);
		return true;
	}

	return false;
}

/**
 * @brief 액터의 AbilitySystemComponent가 지정된 Gameplay Tag를 보유하는지 확인합니다.
 *
 * 액터의 Companion Core Component를 통해 지정된 Gameplay Tag가 존재하는지 확인합니다.
 *
 * @param Actor 태그를 확인할 대상 액터.
 * @param GameplayTag 확인할 Gameplay Tag.
 * @return 태그가 존재하면 true, 그렇지 않으면 false.
 */
bool UGSCBlueprintFunctionLibrary::HasMatchingGameplayTag(AActor* Actor, const FGameplayTag GameplayTag)
{
	// Companion Core Component를 검색합니다.
	UGSCCoreComponent* CCC = GetCompanionCoreComponent(Actor);
	if (!CCC)
	{
		return false;
	}

	// 해당 컴포넌트의 태그 검색 함수를 호출합니다.
	return CCC->HasMatchingGameplayTag(GameplayTag);
}

/**
 * @brief 액터의 AbilitySystemComponent가 지정된 Gameplay Tag들을 하나라도 포함하는지 확인합니다.
 *
 * Companion Core Component를 통해 주어진 태그 컨테이너에 포함된 태그 중 하나라도 보유하고 있는지 확인합니다.
 *
 * @param Actor 태그를 확인할 대상 액터.
 * @param GameplayTags 확인할 Gameplay Tag들이 담긴 컨테이너.
 * @return 하나 이상의 태그가 존재하면 true, 그렇지 않으면 false.
 */
bool UGSCBlueprintFunctionLibrary::HasAnyMatchingGameplayTag(AActor* Actor, const FGameplayTagContainer GameplayTags)
{
	// Companion Core Component를 검색합니다.
	UGSCCoreComponent* CCC = GetCompanionCoreComponent(Actor);
	if (!CCC)
	{
		return false;
	}

	// 해당 컴포넌트의 다중 태그 확인 함수를 호출합니다.
	return CCC->HasAnyMatchingGameplayTags(GameplayTags);
}

/**
 * @brief FGameplayAttribute의 이름을 문자열로 반환합니다.
 *
 * 디버깅 목적으로, 주어진 Gameplay Attribute의 이름을 얻어 문자열로 반환합니다.
 *
 * @param Attribute 확인할 Gameplay Attribute.
 * @return 속성 이름을 포함하는 FString.
 */
FString UGSCBlueprintFunctionLibrary::GetDebugStringFromAttribute(const FGameplayAttribute Attribute)
{
	// FGameplayAttribute의 GetName() 메서드를 호출하여 이름을 반환합니다.
	return Attribute.GetName();
}

/**
 * @brief 주어진 AttributeSet 클래스에 정의된 모든 Gameplay Attribute를 반환합니다.
 *
 * TFieldIterator를 사용하여 AttributeSet 클래스의 모든 FProperty를 순회하고,
 * 각 프로퍼티가 FFloatProperty이거나 Gameplay Attribute 데이터 프로퍼티로 판단되면 OutAttributes 배열에 추가합니다.
 *
 * @param AttributeSetClass 대상 UAttributeSet의 서브클래스.
 * @param OutAttributes 반환될 Gameplay Attribute들을 저장할 배열.
 */
void UGSCBlueprintFunctionLibrary::GetAllAttributes(const TSubclassOf<UAttributeSet> AttributeSetClass, TArray<FGameplayAttribute>& OutAttributes)
{
	// AttributeSetClass의 UClass 포인터를 가져옵니다.
	const UClass* Class = AttributeSetClass.Get();
	// 클래스의 모든 프로퍼티를 순회합니다.
	for (TFieldIterator<FProperty> It(Class); It; ++It)
	{
		// 만약 프로퍼티가 FFloatProperty 타입이면 GameplayAttribute로 생성하여 배열에 추가합니다.
		if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(*It))
		{
			OutAttributes.Push(FGameplayAttribute(FloatProperty));
		}
		// GameplayAttribute 데이터 프로퍼티 여부를 확인하여 배열에 추가합니다.
		else if (FGameplayAttribute::IsGameplayAttributeDataProperty(*It))
		{
			OutAttributes.Push(FGameplayAttribute(*It));
		}
	}
}

/**
 * @brief FGameplayAttribute와 FString 값이 서로 다름을 확인합니다.
 *
 * FGameplayAttribute의 이름과 주어진 문자열을 비교하여 두 값이 일치하지 않으면 true를 반환합니다.
 *
 * @param A 비교할 Gameplay Attribute.
 * @param B 비교할 문자열.
 * @return 두 값이 다르면 true, 같으면 false.
 */
bool UGSCBlueprintFunctionLibrary::NotEqual_GameplayAttributeGameplayAttribute(FGameplayAttribute A, FString B)
{
	// 이름 비교를 통해 불일치 여부를 반환합니다.
	return A.GetName() != B;
}

/**
 * @brief 액터의 AbilitySystemComponent에서 GameplayCue를 실행합니다.
 *
 * 주어진 GameplayCue Tag와 Gameplay Effect Context를 사용하여 액터의 AbilitySystemComponent에서
 * 즉시 실행되는 GameplayCue를 호출합니다.
 *
 * @param Actor 대상 액터.
 * @param GameplayCueTag 실행할 GameplayCue의 태그.
 * @param Context 실행에 필요한 Gameplay Effect Context.
 */
void UGSCBlueprintFunctionLibrary::ExecuteGameplayCueForActor(AActor* Actor, FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context)
{
	// 액터로부터 AbilitySystemComponent를 검색합니다. 두 번째 인자는 강제 검색(true)을 의미합니다.
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		// 검색된 ASC에서 GameplayCue를 실행합니다.
		AbilitySystemComponent->ExecuteGameplayCue(GameplayCueTag, Context);
	}
}

/**
 * @brief 액터의 AbilitySystemComponent에서 추가 매개변수를 포함하여 GameplayCue를 실행합니다.
 *
 * 주어진 GameplayCue Tag와 GameplayCueParameters를 사용하여 더욱 세부적인 설정과 함께
 * GameplayCue를 실행합니다.
 *
 * @param Actor 대상 액터.
 * @param GameplayCueTag 실행할 GameplayCue의 태그.
 * @param GameplayCueParameters 추가 매개변수를 포함한 구조체.
 */
void UGSCBlueprintFunctionLibrary::ExecuteGameplayCueWithParams(AActor* Actor, FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameters)
{
	// AbilitySystemComponent를 검색합니다.
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		// 추가 매개변수를 사용하여 GameplayCue를 실행합니다.
		AbilitySystemComponent->ExecuteGameplayCue(GameplayCueTag, GameplayCueParameters);
	}
}

/**
 * @brief 액터의 AbilitySystemComponent에 지속형 GameplayCue를 추가합니다.
 *
 * 지정된 GameplayCue Tag와 Gameplay Effect Context를 사용하여 액터의 AbilitySystemComponent에
 * 지속적으로 활성화된 GameplayCue를 추가합니다.
 *
 * @param Actor 대상 액터.
 * @param GameplayCueTag 추가할 GameplayCue의 태그.
 * @param Context 실행에 필요한 Gameplay Effect Context.
 */
void UGSCBlueprintFunctionLibrary::AddGameplayCue(AActor* Actor, FGameplayTag GameplayCueTag, FGameplayEffectContextHandle Context)
{
	// ASC를 검색합니다.
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		// 지속형 GameplayCue를 추가합니다.
		AbilitySystemComponent->AddGameplayCue(GameplayCueTag, Context);
	}
}

/**
 * @brief 액터의 AbilitySystemComponent에 추가 매개변수를 포함한 지속형 GameplayCue를 추가합니다.
 *
 * 지정된 GameplayCue Tag와 GameplayCueParameters를 사용하여, 더욱 세부적인 설정과 함께
 * 지속적으로 활성화된 GameplayCue를 추가합니다.
 *
 * @param Actor 대상 액터.
 * @param GameplayCueTag 추가할 GameplayCue의 태그.
 * @param GameplayCueParameter 추가 매개변수를 포함한 GameplayCueParameters 구조체.
 */
void UGSCBlueprintFunctionLibrary::AddGameplayCueWithParams(AActor* Actor, FGameplayTag GameplayCueTag, const FGameplayCueParameters& GameplayCueParameter)
{
	// ASC를 검색합니다.
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		// 추가 매개변수를 사용하여 지속형 GameplayCue를 추가합니다.
		AbilitySystemComponent->AddGameplayCue(GameplayCueTag, GameplayCueParameter);
	}
}

/**
 * @brief 액터의 AbilitySystemComponent에서 특정 GameplayCue를 제거합니다.
 *
 * 지정된 GameplayCue Tag에 해당하는 큐를 액터의 AbilitySystemComponent에서 제거합니다.
 *
 * @param Actor 대상 액터.
 * @param GameplayCueTag 제거할 GameplayCue의 태그.
 */
void UGSCBlueprintFunctionLibrary::RemoveGameplayCue(AActor* Actor, FGameplayTag GameplayCueTag)
{
	// ASC를 강제로 검색합니다.
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		// 해당 GameplayCue를 제거합니다.
		AbilitySystemComponent->RemoveGameplayCue(GameplayCueTag);
	}
}

/**
 * @brief 액터의 AbilitySystemComponent에서 모든 GameplayCue를 제거합니다.
 *
 * 액터에 의해 개별적으로 추가된(게임플레이 효과에 의존하지 않는) 모든 GameplayCue를 제거합니다.
 *
 * @param Actor 대상 액터.
 */
void UGSCBlueprintFunctionLibrary::RemoveAllGameplayCues(AActor* Actor)
{
	// ASC를 강제로 검색합니다.
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor, true);
	if (AbilitySystemComponent)
	{
		// 모든 GameplayCue를 제거합니다.
		AbilitySystemComponent->RemoveAllGameplayCues();
	}
}

/**
 * @brief Ability Set 핸들의 디버그 문자열 표현을 반환합니다.
 *
 * 주어진 FGSCAbilitySetHandle의 내부 정보를 문자열 형태로 변환하여 반환합니다.
 * bVerbose가 true이면 더욱 상세한 정보를 포함한 문자열을 반환합니다.
 *
 * @param InAbilitySetHandle 디버깅할 Ability Set 핸들.
 * @param bVerbose 상세 정보 출력 여부. 기본값은 false.
 * @return Ability Set 상태를 나타내는 FString.
 */
FString UGSCBlueprintFunctionLibrary::DebugAbilitySetHandle(const FGSCAbilitySetHandle& InAbilitySetHandle, const bool bVerbose)
{
	// FGSCAbilitySetHandle의 ToString 메서드를 호출하여 문자열로 변환합니다.
	return InAbilitySetHandle.ToString(bVerbose);
}
