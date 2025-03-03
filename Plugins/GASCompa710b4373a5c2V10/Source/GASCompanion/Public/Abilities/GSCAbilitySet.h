// Copyright 2021-2022 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Engine/DataAsset.h"
#include "GameFeatures/GSCGameFeatureTypes.h"
#include "GSCAbilitySet.generated.h"

// 전방 선언: AbilitySystemComponent 클래스
class UAbilitySystemComponent;

/**
 * @brief Ability Set 적용 후, 생성된 리소스(Abilities, Effects, Attributes 등)를 추적하기 위한 핸들을 저장하는 구조체
 *
 * Ability Set 부여 시, 부여된 Ability와 Effect, AttributeSet, 소유(Owned) 태그 등을 관리하기 위해 사용됩니다.
 * 추후 Ability Set을 제거하려면, 이 구조체에 저장된 정보를 참조하여 제거 작업을 수행합니다.
 */
USTRUCT(BlueprintType)
struct FGSCAbilitySetHandle
{
	GENERATED_BODY()

	/** @brief 부여된 Ability들의 핸들 목록 */
	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> Abilities;

	/** @brief 부여된 GameplayEffect들의 핸들 목록 */
	UPROPERTY()
	TArray<FActiveGameplayEffectHandle> EffectHandles;

	/** @brief 부여된 AttributeSet(속성 세트) 포인터 목록 */
	UPROPERTY()
	TArray<TObjectPtr<UAttributeSet>> Attributes;

	/** @brief Ability Set 적용 시 소유(Owned)한 태그 정보 */
	UPROPERTY()
	FGameplayTagContainer OwnedTags;

	/** @brief 이 Ability Set 핸들이 참조하는 Ability Set의 PathName (디버그용) */
	UPROPERTY(BlueprintReadOnly, Category="Ability Sets")
	FString AbilitySetPathName;

	/** @brief Ability가 부여될 때, 클라이언트 측 입력 바인딩을 위해 등록된 델리게이트 핸들 목록 */
	TArray<FDelegateHandle> InputBindingDelegateHandles;

	/** @brief 기본 생성자 */
	FGSCAbilitySetHandle() = default;

	/**
	 * @brief 현재 핸들이 유효한지(실제 AbilitySet을 가리키는지) 확인
	 * @return AbilitySetPathName이 비어있지 않다면 유효(true), 비어있으면 무효(false)
	 */
	bool IsValid() const
	{
		return !AbilitySetPathName.IsEmpty();
	}

	/**
	 * @brief 핸들 구조체 내 정보를 모두 초기화 (Invalid 상태로 만듦)
	 *
	 * 이미 부여된 리소스를 해제한 뒤 호출해야 합니다.
	 */
	void Invalidate()
	{
		AbilitySetPathName = TEXT("");
		Abilities.Empty();
		EffectHandles.Empty();
		Attributes.Empty();
		OwnedTags.Reset();
	}

	/**
	 * @brief 핸들에 대한 정보(AbilitySetPathName, Abilities, Effects 등)를 문자열로 반환 (디버그용)
	 * @param bVerbose true면 Abilities, Effects, Attributes 등의 상세 목록까지 문자열에 포함
	 * @return 핸들의 정보를 담은 FString
	 */
	FString ToString(const bool bVerbose = false) const
	{
		TArray<FString> Results;
		Results.Add(FString::Printf(
			TEXT("AbilitySetPathName: %s, Abilities Handles: %d, Effect Handles: %d, Attribute Sets: %d, Owned Tags: %d"),
			*AbilitySetPathName,
			Abilities.Num(),
			EffectHandles.Num(),
			Attributes.Num(),
			OwnedTags.Num()
		));

		// bVerbose가 false이면 요약 정보만 반환
		if (!bVerbose)
		{
			return FString::Join(Results, LINE_TERMINATOR);
		}

		// bVerbose가 true이면 세부 목록도 출력
		Results.Add(FString::Printf(TEXT("Abilities Handles: %d"), Abilities.Num()));
		for (const FGameplayAbilitySpecHandle& AbilityHandle : Abilities)
		{
			Results.Add(FString::Printf(TEXT("\t - Ability Handle: %s"), *AbilityHandle.ToString()));
		}

		Results.Add(FString::Printf(TEXT("Effect Handles: %d"), EffectHandles.Num()));
		for (const FActiveGameplayEffectHandle& EffectHandle : EffectHandles)
		{
			Results.Add(FString::Printf(TEXT("\t - Effect Handle: %s"), *EffectHandle.ToString()));
		}
		
		Results.Add(FString::Printf(TEXT("Attribute Sets: %d"), Attributes.Num()));
		for (const UAttributeSet* AttributeSet : Attributes)
		{
			Results.Add(FString::Printf(TEXT("\t - Attribute Set: %s"), *GetNameSafe(AttributeSet)));
		}

		Results.Add(FString::Printf(TEXT("Owned Tags: %d"), OwnedTags.Num()));
		Results.Add(FString::Printf(TEXT("\t - Owned Tags: %s"), *OwnedTags.ToStringSimple()));

		return FString::Join(Results, LINE_TERMINATOR);
	}

	friend bool operator==(const FGSCAbilitySetHandle& LHS, const FGSCAbilitySetHandle& RHS)
	{
		return LHS.AbilitySetPathName == RHS.AbilitySetPathName;
	}

	friend bool operator!=(const FGSCAbilitySetHandle& LHS, const FGSCAbilitySetHandle& RHS)
	{
		return !(LHS == RHS);
	}
};


/**
 * @brief AbilitySystemComponent에 적용할 Ability, Attribute, Effect, 태그 등을 한 곳에서 정의하는 DataAsset
 *
 * 이 클래스(AbilitySet)는 여러 Ability, AttributeSet, GameplayEffect 및 소유(Owned) 태그를 묶어,
 * 한 번에 쉽게 ASC에 부여하고 제거할 수 있도록 관리하는 데이터 자산(Data Asset)입니다.
 */
UCLASS(BlueprintType)
class GASCOMPANION_API UGSCAbilitySet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** @brief 기본 생성자 */
	UGSCAbilitySet() = default;

	// ----------------------------------------------------------------------
	//  Ability / Attribute / Effect Definitions
	// ----------------------------------------------------------------------

	/** @brief ASC 초기화 시 자동으로 부여할 Ability 목록 (input binding 포함 가능) */
	UPROPERTY(EditDefaultsOnly, Category="Abilities", meta=(TitleProperty=AbilityType))
	TArray<FGSCGameFeatureAbilityMapping> GrantedAbilities;

	/** @brief ASC 초기화 시 자동으로 부여할 AttributeSet 목록 (초기화 data table 포함 가능) */
	UPROPERTY(EditDefaultsOnly, Category="Attributes", meta=(TitleProperty=AttributeSet))
	TArray<FGSCGameFeatureAttributeSetMapping> GrantedAttributes;

	/** @brief ASC 초기화 시 자동으로 적용할 GameplayEffect 목록 (예: 상태 이상, 버프 등) */
	UPROPERTY(EditDefaultsOnly, Category="Effects", meta=(TitleProperty=EffectType))
	TArray<FGSCGameFeatureGameplayEffectMapping> GrantedEffects;
	
	/** @brief 이 AbilitySet이 적용될 때 ASC에 부여될 태그들 */
	UPROPERTY(EditDefaultsOnly, Category="Owned Gameplay Tags")
	FGameplayTagContainer OwnedTags;


	// ----------------------------------------------------------------------
	//  Public Methods
	// ----------------------------------------------------------------------

	/**
	 * @brief 이 Ability Set(자신)을 UAbilitySystemComponent에 적용하여 Ability, Attribute, Effect, 태그 등을 부여
	 *
	 * @param InASC 부여할 대상 ASC
	 * @param OutAbilitySetHandle 부여 결과로 얻은 AbilitySet 핸들(추후 제거 시 필요)
	 * @param OutErrorText 부여 실패 시 오류 메시지가 저장될 FText 포인터 (선택)
	 * @param bShouldRegisterCoreDelegates CoreComponent 등록 로직 수행 여부 (true면 부여 후에 CoreComponent Delegates 등록 재시도)
	 * @return 부여 성공 시 true, 실패 시 false
	 */
	bool GrantToAbilitySystem(UAbilitySystemComponent* InASC, FGSCAbilitySetHandle& OutAbilitySetHandle, FText* OutErrorText = nullptr, const bool bShouldRegisterCoreDelegates = true) const;

	/**
	 * @brief 이 Ability Set(자신)을 특정 액터(ASC 소유)에 적용하여 Ability, Attribute, Effect, 태그 등을 부여
	 *
	 * @param InActor ASC를 가지는 액터
	 * @param OutAbilitySetHandle 부여 결과로 얻은 AbilitySet 핸들
	 * @param OutErrorText 부여 실패 시 오류 메시지가 저장될 FText 포인터 (선택)
	 * @return 부여 성공 시 true, 실패 시 false
	 */
	bool GrantToAbilitySystem(const AActor* InActor, FGSCAbilitySetHandle& OutAbilitySetHandle, FText* OutErrorText = nullptr) const;

	/**
	 * @brief 이 Ability Set으로 부여되었던 리소스(Ability, Attribute, Effect 등)를 ASC로부터 제거
	 * 
	 * @param InASC 제거 대상 ASC
	 * @param InAbilitySetHandle 제거할 AbilitySet 핸들
	 * @param OutErrorText 제거 실패 시 오류 메시지가 저장될 FText 포인터 (선택)
	 * @param bShouldRegisterCoreDelegates 제거 후 CoreComponent 재등록 로직 수행 여부
	 * @return 제거 성공 시 true, 실패 시 false
	 */
	static bool RemoveFromAbilitySystem(UAbilitySystemComponent* InASC, FGSCAbilitySetHandle& InAbilitySetHandle, FText* OutErrorText = nullptr, const bool bShouldRegisterCoreDelegates = true);

	/**
	 * @brief 이 Ability Set으로 부여되었던 리소스(Ability, Attribute, Effect 등)를 액터(ASC 소유)로부터 제거
	 * 
	 * @param InActor ASC를 가지는 액터
	 * @param InAbilitySetHandle 제거할 AbilitySet 핸들
	 * @param OutErrorText 제거 실패 시 오류 메시지가 저장될 FText 포인터 (선택)
	 * @return 제거 성공 시 true, 실패 시 false
	 */
	static bool RemoveFromAbilitySystem(const AActor* InActor, FGSCAbilitySetHandle& InAbilitySetHandle, FText* OutErrorText = nullptr);

	/**
	 * @brief 이 AbilitySet이 입력 바인딩(UGSCAbilityInputBindingComponent)이 필요한지 여부
	 *
	 * GrantedAbilities 중 하나라도 입력 액션(InputAction)이 정의되어 있으면 true를 반환
	 */
	bool HasInputBinding() const;


protected:

	/** 
	 * @brief 아바타 액터가 GSCCoreComponent를 가지고 있다면, 속성 추가 등을 위한 델리게이트 등록 로직 수행
	 *
	 * ASC에 AbilitySet을 적용 후, CoreComponent가 존재할 경우 이를 재등록하여 Attribute나 각종 Delegate를 갱신합니다.
	 * 
	 * @param InASC 작업 대상 ASC
	 */
	static void TryRegisterCoreComponentDelegates(UAbilitySystemComponent* InASC);
	
	/**
	 * @brief 아바타 액터가 GSCCoreComponent를 가지고 있다면, 언레지스터(del) 로직 수행
	 *
	 * AbilitySet 제거 시, CoreComponent에 등록된 델리게이트 등을 해제합니다.
	 * 
	 * @param InASC 작업 대상 ASC
	 */
	static void TryUnregisterCoreComponentDelegates(UAbilitySystemComponent* InASC);
};


