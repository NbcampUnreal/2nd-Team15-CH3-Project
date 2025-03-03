// Copyright 2020 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GSCTemplate_GameplayEffectDefinition.generated.h"

/**
 * @brief Gameplay Effect 템플릿의 부모 클래스.
 *
 * 이 클래스는 Gameplay Effect 템플릿 전용으로 사용됩니다.
 * 템플릿은 고유한 종류로, 이 클래스의 Class Default Object(CDO)를 기반으로 다른 Gameplay Effect를 생성하는 용도로 사용되며,
 * 직접적으로 사용되거나 상속되어 사용되지는 않습니다.
 *
 * UGameplayEffect의 자식 클래스가 아니라 UObject를 상속받으며, UGameplayEffect와 동일한 속성들을 공유합니다.
 *
 * 이 클래스의 존재 목적은 표준 Gameplay Effect 목록에 나타나지 않고,
 * 프로젝트 설정에서 구성 가능한 GE 템플릿 Blueprint를 생성할 수 있도록 하기 위함입니다.
 * 템플릿으로부터 새로운 GE를 생성하면, 템플릿에 정의된 속성들을 기반으로 실제 UGameplayEffect Blueprint가 생성됩니다.
 */
UCLASS(Abstract, Blueprintable)
class GASCOMPANION_API UGSCTemplate_GameplayEffectDefinition : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * @brief 기본 생성자.
	 *
	 * 이 생성자는 객체 초기화 시 기본 멤버들을 설정합니다.
	 */
	UGSCTemplate_GameplayEffectDefinition();

	//~ Begin UObject interface
	/**
	 * @brief 객체 초기화 후 호출되는 함수.
	 *
	 * PostInitProperties()는 UObject가 생성된 후, 기본 속성 초기화가 완료되었을 때 호출되며,
	 * 이 클래스에서는 추가적인 초기화 작업을 수행합니다.
	 */
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	/**
	 * @brief 에디터에서 속성 값이 변경된 후 호출되는 함수.
	 *
	 * 이 함수는 에디터 상에서 속성 값이 변경될 때, 상속된 태그 컨테이너들의 속성도 업데이트하도록 합니다.
	 *
	 * @param PropertyChangedEvent 변경된 속성 정보를 담은 이벤트 구조체.
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	//~ End UObject interface
	
	/** 
	 * @brief 이 효과의 지속 시간 정책.
	 *
	 * Gameplay Effect가 얼마나 오래 지속될지를 결정하는 정책입니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category=GameplayEffect)
	EGameplayEffectDurationType DurationPolicy;

	/**
	 * @brief 효과의 지속 시간(초).
	 *
	 * 0.0이면 순간 효과, -1.0이면 무한 지속 효과를 의미합니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category=GameplayEffect)
	FGameplayEffectModifierMagnitude DurationMagnitude;

	/**
	 * @brief 효과가 주기적으로 실행되는 경우의 주기(초).
	 *
	 * 0.0이면 주기적 실행이 없는 효과를 의미합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Period)
	FScalableFloat	Period;
	
	/**
	 * @brief 효과가 적용될 때 바로 실행되고, 이후 매 주기마다 실행할지 여부.
	 *
	 * true이면 효과 적용 시점과 이후 주기마다 실행되며,
	 * false이면 첫 주기가 지난 후에만 실행됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Period)
	bool bExecutePeriodicEffectOnApplication;

	/**
	 * @brief 효과 주기 중 억제(inhibition) 해제 정책.
	 *
	 * 효과의 주기가 재설정되지 않는 등의 세부 정책을 결정합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Period)
	EGameplayEffectPeriodInhibitionRemovedPolicy PeriodicInhibitionPolicy;

	/**
	 * @brief 대상에게 영향을 미칠 수정자들의 배열.
	 *
	 * 이 배열에 포함된 각 수정자는 효과가 대상에게 적용될 때, 대상의 속성을 변경하는 데 사용됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=GameplayEffect, meta=(TitleProperty=Attribute))
	TArray<FGameplayModifierInfo> Modifiers;

	/**
	 * @brief 효과 실행 정의 배열.
	 *
	 * 효과가 적용될 때 실행될 추가 로직(예: 복잡한 계산, 조건부 효과 적용 등)을 정의합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayEffect)
	TArray<FGameplayEffectExecutionDefinition> Executions;

	/**
	 * @brief 대상에게 효과가 적용될 확률 (0.0은 적용 안됨, 1.0은 항상 적용).
	 */
	UPROPERTY(EditDefaultsOnly, Category=Application, meta=(GameplayAttribute="True"))
	FScalableFloat ChanceToApplyToTarget;

	/**
	 * @brief 효과 적용 시 필요한 추가 요구사항을 정의하는 클래스 배열.
	 */
	UPROPERTY(EditDefaultsOnly, Category=Application, DisplayName="Application Requirement")
	TArray<TSubclassOf<UGameplayEffectCustomApplicationRequirement> > ApplicationRequirements;

	/**
	 * @brief 이 효과가 적용될 경우, 대상에게 추가로 적용될 조건부 GameplayEffect들을 정의하는 배열.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GameplayEffect)
	TArray<FConditionalGameplayEffect> ConditionalGameplayEffects;

	/**
	 * @brief 스택이 오버플로우될 때 적용할 효과들의 배열.
	 *
	 * 스택 수치가 제한을 초과하면, 오버플로우 적용 시도와 관계없이 이 효과들이 대상에게 적용됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Overflow)
	TArray<TSubclassOf<UGameplayEffect>> OverflowEffects;

	/**
	 * @brief 스택 최대치에 도달했을 때 추가 스택 적용 시도가 실패하도록 할지 여부.
	 *
	 * true이면 추가 스택 적용 시도가 실패하여, 효과의 지속 시간 및 컨텍스트가 갱신되지 않습니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Overflow)
	bool bDenyOverflowApplication;

	/**
	 * @brief 오버플로우 발생 시, 전체 효과 스택을 초기화할지 여부.
	 *
	 * true이면 오버플로우가 발생할 때 효과 스택 전체가 제거됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Overflow, meta=(EditCondition="bDenyOverflowApplication"))
	bool bClearStackOnOverflow;

	/**
	 * @brief 효과가 강제 제거 등으로 조기 만료될 때 적용할 효과들의 배열.
	 *
	 * 이 효과는 지속 시간이 있는 효과에 한해서 적용되며, 효과가 강제 제거될 때 추가로 적용됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Expiration)
	TArray<TSubclassOf<UGameplayEffect>> PrematureExpirationEffectClasses;

	/**
	 * @brief 효과가 자연스럽게 만료될 때 적용할 효과들의 배열.
	 *
	 * 이 효과 역시 지속 시간이 있는 효과에 한해서 적용됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Expiration)
	TArray<TSubclassOf<UGameplayEffect>> RoutineExpirationEffectClasses;

	/**
	 * @brief 효과 수정자가 성공적으로 적용되어야만 GameplayCue가 발생하도록 할지 여부.
	 *
	 * true이면 GE 수정자가 성공적으로 적용되어야만 효과와 관련된 시각적/청각적 반응이 발생합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Display)
	bool bRequireModifierSuccessToTriggerCues;

	/**
	 * @brief 스택 효과의 경우, 첫 번째 인스턴스에 대해서만 GameplayCue를 발생시킬지 여부.
	 *
	 * true이면 중복된 스택 효과에서는 GameplayCue가 발생하지 않습니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = Display)
	bool bSuppressStackingCues;

	/**
	 * @brief 효과와 관련된 비시뮬레이션 반응(예: 사운드, 파티클 등)을 트리거할 GameplayCue들의 배열.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Display)
	TArray<FGameplayEffectCue>	GameplayCues;

	/**
	 * @brief 효과의 UI 표현을 위한 데이터.
	 *
	 * 이 데이터에는 텍스트, 아이콘 등의 UI 관련 정보가 포함되며, 서버 전용 빌드에서는 제공되지 않습니다.
	 */
	UPROPERTY(EditDefaultsOnly, Instanced, BlueprintReadOnly, Category = Display)
	TObjectPtr<class UGameplayEffectUIData> UIData;

	// ----------------------------------------------------------------------
	//	Tag Containers
	// ----------------------------------------------------------------------
	
	/**
	 * @brief 이 GameplayEffect가 보유한 태그들.
	 *
	 * 이 태그들은 효과가 자체적으로 가지고 있는 태그로, 대상에게 부여되지 않습니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags, meta = (DisplayName = "GameplayEffectAssetTag", Categories="GameplayEffectTagsCategory"))
	FInheritedTagContainer InheritableGameplayEffectTags;
	
	/**
	 * @brief 효과가 대상에게 적용될 때 함께 부여되는 태그들.
	 *
	 * 이 태그들은 효과가 대상에게 적용될 때 부여되며, 소유자에게 영향을 미칩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags, meta=(DisplayName="GrantedTags", Categories="OwnedTagsCategory"))
	FInheritedTagContainer InheritableOwnedTagsContainer;
	
	/**
	 * @brief 효과 적용 후, 효과가 "켜짐" 또는 "꺼짐" 상태를 결정하는 태그 요구사항.
	 *
	 * 이 요구사항은 효과가 적용된 후, 효과의 활성화 여부를 판단하는 데 사용됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags, meta=(Categories="OngoingTagRequirementsCategory"))
	FGameplayTagRequirements OngoingTagRequirements;

	/**
	 * @brief 효과가 대상에 적용되기 위한 태그 요구사항.
	 *
	 * 이 요구사항은 효과 적용 시 통과해야 하는 조건이며, 요구사항을 충족하지 못하면 효과가 적용되지 않습니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags, meta=(Categories="ApplicationTagRequirementsCategory"))
	FGameplayTagRequirements ApplicationTagRequirements;

	/**
	 * @brief 효과를 제거하기 위한 태그 요구사항.
	 *
	 * 이 요구사항이 충족되면, 효과가 제거되며 효과의 적용도 차단됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags, meta=(Categories="ApplicationTagRequirementsCategory"))
	FGameplayTagRequirements RemovalTagRequirements;

	/**
	 * @brief 효과 적용 시, 이 태그를 보유한 모든 GameplayEffect들이 제거됩니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags, meta=(Categories="RemoveTagRequirementsCategory"))
	FInheritedTagContainer RemoveGameplayEffectsWithTags;

	/**
	 * @brief 소유자에게 부여되는, 특정 소스 태그에 대한 면역성을 나타내는 태그 요구사항.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Immunity, meta = (DisplayName = "GrantedApplicationImmunityTags", Categories="GrantedApplicationImmunityTagsCategory"))
	FGameplayTagRequirements GrantedApplicationImmunityTags;

	/**
	 * @brief 이 쿼리에 해당하는 GameplayEffect에 대해 면역성을 부여합니다.
	 *
	 * 쿼리는 GrantedApplicationImmunityTags보다 강력하지만, 처리 속도는 약간 느릴 수 있습니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Immunity)
	FGameplayEffectQuery GrantedApplicationImmunityQuery;

	/**
	 * @brief 효과 적용 시, 활성화된 효과들 중 이 쿼리에 해당하는 효과들을 제거합니다.
	 *
	 * 이 쿼리는 RemoveGameplayEffectsWithTags보다 강력하지만, 처리 속도는 약간 느릴 수 있습니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Tags, meta = (DisplayAfter = "RemovalTagRequirements"))
	FGameplayEffectQuery RemoveGameplayEffectQuery;

	// ----------------------------------------------------------------------
	//	Stacking
	// ----------------------------------------------------------------------
	
	/**
	 * @brief 이 GameplayEffect가 동일한 효과의 다른 인스턴스와 어떻게 스택되는지 결정하는 옵션.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Stacking)
	EGameplayEffectStackingType	StackingType;

	/**
	 * @brief 스택 제한 개수.
	 *
	 * StackingType에 따라 효과가 쌓일 수 있는 최대 개수를 정의합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Stacking)
	int32 StackLimitCount;

	/**
	 * @brief 스택 효과의 지속 시간 갱신 정책.
	 *
	 * 효과가 스택될 때, 기존 지속 시간을 갱신할지 여부를 결정합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Stacking)
	EGameplayEffectStackingDurationPolicy StackDurationRefreshPolicy;

	/**
	 * @brief 스택 효과의 주기(Period) 재설정 정책.
	 *
	 * 효과가 스택될 때, 효과의 주기를 재설정할지 여부를 결정합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Stacking)
	EGameplayEffectStackingPeriodPolicy StackPeriodResetPolicy;

	/**
	 * @brief 스택 효과의 만료 정책.
	 *
	 * 스택된 효과의 지속 시간이 만료될 때, 어떻게 처리할지를 결정합니다.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Stacking)
	EGameplayEffectStackingExpirationPolicy StackExpirationPolicy;

	// ----------------------------------------------------------------------
	//	Granted abilities
	// ----------------------------------------------------------------------
	
	/**
	 * @brief 이 효과가 적용될 때 대상에게 부여될 능력들의 스펙 정의 배열.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Granted Abilities")
	TArray<FGameplayAbilitySpecDef>	GrantedAbilities;

	/**
	 * @brief 템플릿의 Class Default Object에서 GameplayEffect에 필요한 속성들을 복사합니다.
	 *
	 * 이 정적 함수는 템플릿으로부터 생성된 CDO(클래스 기본 객체)의 속성들을 실제 적용할 UGameplayEffect CDO로 복사합니다.
	 * 이를 통해 템플릿에 설정한 효과 특성이 실제 GameplayEffect에 반영됩니다.
	 *
	 * @param CDO 실제 GameplayEffect Blueprint의 Class Default Object.
	 * @param TemplateCDO 템플릿으로 사용된 UGSCTemplate_GameplayEffectDefinition의 Class Default Object.
	 */
	static void CopyProperties(UGameplayEffect* CDO, UGSCTemplate_GameplayEffectDefinition* TemplateCDO);
};


