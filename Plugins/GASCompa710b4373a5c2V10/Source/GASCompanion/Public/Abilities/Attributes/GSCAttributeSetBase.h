#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"

#include "GSCAttributeSetBase.generated.h"

// 전방 선언
class UGSCCoreComponent;
class AGSCCharacterBase;
struct FGameplayTagContainer;

/**
 * @brief FGameplayEffectModCallbackData에서 추출한 정보들을 담는 구조체
 *
 * - GameplayEffect가 실행될 때, 속성 변경 전/후에 필요한 다양한 정보를 캐싱하여
 *   AttributeSet이 로직 처리 시 활용할 수 있게 합니다.
 * - 예: 소스 액터(Instigator), 타겟 액터, ASC, 플레이어 컨트롤러, Pawn 캐스팅, 델타 값 등
 */
USTRUCT()
struct FGSCAttributeSetExecutionData
{
	GENERATED_BODY()

	/** @brief 데미지 또는 효과를 유발한 액터(Instigator)를 나타냅니다. */
	UPROPERTY()
	TObjectPtr<AActor> SourceActor = nullptr;

	/** @brief 효과가 적용될 타겟 액터(해당 AttributeSet을 보유한 액터)입니다. */
	UPROPERTY()
	TObjectPtr<AActor> TargetActor = nullptr;

	/** @brief 데미지 또는 효과를 유발한 액터의 ASC(Ability System Component) */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceASC = nullptr;

	/** @brief 소스 액터(Instigator)에 연결된 GAS Companion 코어 컴포넌트 (UGSCCoreComponent) */
	UPROPERTY()
	TObjectPtr<UGSCCoreComponent> SourceCoreComponent = nullptr;

	/** @brief 타겟 액터에 연결된 GAS Companion 코어 컴포넌트 (UGSCCoreComponent) */
	UPROPERTY()
	TObjectPtr<UGSCCoreComponent> TargetCoreComponent = nullptr;

	/** @brief 소스 액터(Instigator)의 플레이어 컨트롤러 */
	UPROPERTY()
	TObjectPtr<APlayerController> SourceController = nullptr;

	/** @brief 타겟 액터의 플레이어 컨트롤러 */
	UPROPERTY()
	TObjectPtr<APlayerController> TargetController = nullptr;

	/** @brief 소스 액터(Instigator)를 Pawn으로 캐스팅한 포인터 (예: 캐릭터/AI 등) */
	UPROPERTY()
	TObjectPtr<APawn> SourcePawn = nullptr;

	/** @brief 타겟 액터를 Pawn으로 캐스팅한 포인터 */
	UPROPERTY()
	TObjectPtr<APawn> TargetPawn = nullptr;

	/** @brief GameplayEffect를 생성할 때 사용된 UObject (예: Weapon, Item 등) */
	UPROPERTY()
	TObjectPtr<UObject> SourceObject = nullptr;

	/** @brief GameplayEffect가 적용된 컨텍스트 (누가, 어떻게 적용했는지 등의 정보 포함) */
	FGameplayEffectContextHandle Context;

	/** @brief 소스 태그(Instigator)에 대한 정보가 포함된 태그 컨테이너 */
	FGameplayTagContainer SourceTags;

	/** @brief GameplayEffectSpec 생성 시 적용된 에셋 태그들 */
	FGameplayTagContainer SpecAssetTags;

	/** @brief 이전 값과 새 값의 차이(Delta Value). Additive 연산일 경우 유효한 값 */
	float DeltaValue;
};

// 매크로: GAS Companion 등에서 속성 접근 시 자주 사용하는 매크로들의 단축키
// - ATTRIBUTE_ACCESSORS(클래스명, 속성명)을 사용하면,
//   해당 속성에 대해 Get/Set/Init 메서드를 자동으로 정의하는 코드를 생성합니다.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * @brief 기본 AttributeSet 클래스 (UGSCAttributeSetBase)
 *
 * 이 클래스는 UAttributeSet을 상속받아, 프로젝트 전반에서 속성 관련 공통 로직(예: 속성 클램프, PostGameplayEffectExecute 후 처리 등)을 구현하기 위한 베이스 클래스입니다.
 *
 * 주요 기능:
 * - PreAttributeChange: 속성 변경 전 호출되어, 필요 시 비례 조정 등 추가 로직 실행.
 * - PostGameplayEffectExecute: GameplayEffect 적용 후 호출되어, 코어 컴포넌트를 통한
 *   이벤트 전달 등 추가 처리를 수행.
 * - AdjustAttributeForMaxChange: 최대 속성 변경 시 현재 속성 값을 비례적으로 조정.
 * - GetExecutionDataFromMod: GameplayEffectModCallbackData에서 필요한 정보를 추출.
 *
 * 구체적인 속성(Health, Stamina 등)은 이 클래스에 직접 정의하지 않고,
 * 파생 클래스에서 확장하여 사용합니다.
 *
 * GAS Companion Core와 연동하여, 속성 변경 시 UGSCCoreComponent를 통해 추가 이벤트를 호출할 수 있습니다.
 */
UCLASS(Abstract)
class GASCOMPANION_API UGSCAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	//----------------------------------------
	// 생성자
	//----------------------------------------

	/** @brief 기본 생성자: AttributeSet의 기본 초기값을 설정합니다. */
	UGSCAttributeSetBase();

	/**
	 * @brief 속성 변경 전 처리 함수 (PreAttributeChange)
	 *
	 * 이 함수는 속성의 값이 변경되기 전에 호출되며, 변경될 새 값(NewValue)을 조정할 수 있는 기회를 제공합니다.
	 * 예를 들어, 최대 속성(예: MaxHealth)이 변경될 경우, 현재 속성(예: Health)이 동일한 비율로 조정되도록 할 수 있습니다.
	 *
	 * @param Attribute 변경될 속성을 나타내는 FGameplayAttribute 객체
	 * @param NewValue 변경 후 적용될 새 값 (참조형으로 전달되어 수정 가능)
	 *
	 * @note 이 함수는 부모 클래스의 PreAttributeChange를 호출한 후, 추가적으로
	 * GAS Companion Core 컴포넌트에 해당 변경을 전달하여, 추가 로직(예: 비례 조정)을 수행합니다.
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	 * @brief GameplayEffect 적용 후 호출되는 함수.
	 *
	 * GameplayEffect가 적용되어 속성 값이 실제 변경된 후 호출됩니다.
	 * 이 함수에서는 변경된 효과에 따른 추가 처리를 할 수 있으며, GAS Companion Core와 연동하여 코어 컴포넌트의 이벤트를 호출할 수 있습니다.
	 *
	 * @param Data GameplayEffect 적용에 대한 상세 정보 (이전 값, 새 값, Instigator 등)
	 */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/**
	 * @brief 네트워크 복제를 위해, 복제할 속성들을 지정합니다.
	 *
	 * 이 함수는 언리얼 엔진의 네트워크 시스템에 의해 호출되며, 이 AttributeSet에 포함된 속성 중 어떤 것을 네트워크를 통해 동기화할지 결정합니다.
	 *
	 * @param OutLifetimeProps 네트워크 복제 대상 속성 목록이 저장될 배열
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//----------------------------------------
	// 클램프 및 태그 관련 함수
	//----------------------------------------

	/**
	 * @brief 특정 속성의 최소 클램프 값을 반환합니다.
	 *
	 * 기본적으로는 0.f를 반환하지만, 서브 클래스에서 오버라이드하여 다른 최소값을 지정할 수 있습니다.
	 *
	 * @param Attribute 클램프 값을 조회할 속성
	 * @return 클램프할 최소값
	 */
	virtual float GetClampMinimumValueFor(const FGameplayAttribute& Attribute);

	/**
	 * @brief GameplayEffectSpec에서 소스 태그를 추출하여 반환합니다.
	 *
	 * @param Data GameplayEffect 적용 콜백 데이터
	 * @return 소스 태그를 담은 FGameplayTagContainer 참조
	 */
	virtual const FGameplayTagContainer& GetSourceTagsFromContext(const FGameplayEffectModCallbackData& Data);

	//----------------------------------------
	// 템플릿 함수: 소스 및 타겟 액터 추출
	//----------------------------------------

	/**
	 * @brief FGameplayEffectModCallbackData에서 소스(Instigator)와 타겟(Target) 액터를 추출하여 템플릿 타입으로 반환합니다.
	 *
	 * 이 함수는 데이터의 컨텍스트(Context)를 이용하여 소스와 타겟 액터를 가져옵니다.
	 * - 타겟 액터는 Data.Target의 AbilityActorInfo에 저장된 AvatarActor를 사용합니다.
	 * - 소스 액터는 Context에 EffectCauser가 명시되어 있다면 그것을 사용하고,
	 *   그렇지 않으면 원본 Instigator의 AbilityActorInfo의 AvatarActor를 사용합니다.
	 *
	 * @tparam TReturnType 반환할 액터의 타입 (예: AActor, APawn 등)
	 * @param Data GameplayEffect 적용에 사용된 콜백 데이터
	 * @param SourceActor TReturnType으로 캐스팅된 소스 액터 (출력)
	 * @param TargetActor TReturnType으로 캐스팅된 타겟 액터 (출력)
	 */
	template <class TReturnType>
	void GetSourceAndTargetFromContext(const FGameplayEffectModCallbackData& Data, TReturnType*& SourceActor, TReturnType*& TargetActor)
	{
		const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
		UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();

		// 타겟 액터 설정: Data.Target의 AbilityActorInfo가 유효하면 AvatarActor를 캐스팅하여 할당
		if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
		{
			TargetActor = Cast<TReturnType>(Data.Target.AbilityActorInfo->AvatarActor.Get());
		}

		// 소스 액터 설정: Instigator ASC의 AbilityActorInfo가 유효하면,
		// Context에 EffectCauser가 있을 경우 우선 그것을 사용, 그렇지 않으면 AvatarActor 사용
		if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
		{
			if (Context.GetEffectCauser())
			{
				SourceActor = Cast<TReturnType>(Context.GetEffectCauser());
			}
			else
			{
				SourceActor = Cast<TReturnType>(Source->AbilityActorInfo->AvatarActor.Get());
			}
		}
	}

	//----------------------------------------
	// 속성 값 조정 함수
	//----------------------------------------

	/**
	 * @brief 특정 속성의 최대값이 변경될 때, 그에 따른 연관 속성의 값을 비례적으로 조정합니다.
	 *
	 * 예를 들어, 최대 체력(MaxHealth)이 증가하면 현재 체력(Health)도 기존 비율을 유지하도록 조정됩니다.
	 * 이를 통해 캐릭터의 상대적 체력 상태를 유지할 수 있습니다.
	 *
	 * @param AffectedAttribute 변경 대상 속성의 FGameplayAttributeData (예: Health)
	 * @param MaxAttribute 해당 속성의 최대값을 나타내는 FGameplayAttributeData (예: MaxHealth)
	 * @param NewMaxValue 새로 적용될 최대값
	 * @param AffectedAttributeProperty 실제 FGameplayAttribute (속성 이름 정보 제공)
	 */
	virtual void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute,
	                                         const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const;

protected:
	//----------------------------------------
	// 내부 데이터 추출 함수
	//----------------------------------------

	/**
	 * @brief FGameplayEffectModCallbackData에서 필요한 정보를 추출하여, FGSCAttributeSetExecutionData 구조체에 채워 반환합니다.
	 *
	 * 이 함수는 PostGameplayEffectExecute 등의 상황에서 호출되어, 효과의 적용 맥락, 소스/타겟 액터, 태그, 델타 값 등의 정보를 취합합니다.
	 *
	 * @param Data GameplayEffect 적용 콜백 데이터
	 * @param OutExecutionData 추출된 정보가 저장될 FGSCAttributeSetExecutionData 구조체 참조
	 */
	virtual void GetExecutionDataFromMod(const FGameplayEffectModCallbackData& Data, OUT FGSCAttributeSetExecutionData& OutExecutionData);
};
