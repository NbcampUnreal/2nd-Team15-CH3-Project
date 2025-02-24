// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

//=============================================================================
// GAS Companion - UGSCCoreComponent
//=============================================================================
// 본 파일은 GAS Companion 시스템의 핵심 컴포넌트인 UGSCCoreComponent를 정의합니다.
// UGSCCoreComponent는 Ability System Component(ASC)와 상호작용하여
// 게임 캐릭터의 속성(Attribute) 및 능력(Ability) 관련 이벤트를 관리하고,
// 블루프린트에서 손쉽게 이벤트를 바인딩할 수 있도록 델리게이트를 제공합니다.
//=============================================================================

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UI/GSCUWHud.h" // HUD 관련 기능(게임 UI) 포함
#include "GSCCoreComponent.generated.h"

//////////////////////////////////////////////////////////////////////////
// 전방 선언
//////////////////////////////////////////////////////////////////////////
class UGSCGameplayAbility;
class UGameplayAbility;
class UGameplayEffect;
class UAbilitySystemComponent;
class UGSCAttributeSetBase;
struct FGameplayAbilitySpecHandle;

//////////////////////////////////////////////////////////////////////////
// 구조체 선언
//////////////////////////////////////////////////////////////////////////

/**
 * @brief GameplayEffect 실행 후 블루프린트 이벤트에 전달되는 데이터 구조체
 *
 * - 속성 값 변화(Delta), 해당 AttributeSet, ASC, 그리고 최소 클램프 값 정보를 포함합니다.
 */
USTRUCT(BlueprintType)
struct FGSCGameplayEffectExecuteData
{
	GENERATED_BODY()

	/** 이벤트가 호출된 AttributeSet (게임플레이 속성 집합) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AttributeSetPayload)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;

	/** 해당 AttributeSet을 소유한 AbilitySystemComponent */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AttributeSetPayload)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	/** 이전 값에서 새로운 값으로의 변화량 (Delta) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AttributeSetPayload)
	float DeltaValue = 0.f;

	/** 속성의 최소 허용값 (Clamp MinimumValue)이 정의되어 있을 경우 해당 값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AttributeSetPayload)
	float ClampMinimumValue = 0.f;
};

//================================================================================
// 델리게이트 선언
// 아래의 DECLARE_DYNAMIC_MULTICAST_DELEGATE* 매크로들은 브루프린트에서 이벤트를 바인딩할 수 있도록
// 다양한 게임플레이 상황(예: 죽음, 능력 활성/종료, 속성 변화 등)에 대한 델리게이트를 정의합니다.
//================================================================================

/** 캐릭터 사망 이벤트 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGSCOnDeath);

/** AbilityActorInfo 초기화 후 호출되는 이벤트 델리게이트 (ASC 초기화 후) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGSCOnInitAbilityActorInfoCore);

/** 기본 속성 변화 이벤트: DeltaValue와 이벤트 태그 정보를 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGSCOnDefaultAttributeChange, float, DeltaValue, const struct FGameplayTagContainer, EventTags);

/** 특정 속성의 변화 이벤트: 변경된 속성과 변화량, 이벤트 태그를 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSCOnAttributeChange, FGameplayAttribute, Attribute, float, DeltaValue, const struct FGameplayTagContainer, EventTags);

/** 속성 변경 전 이벤트: 속성 변경 전 AttributeSet, 속성, 새 값 정보를 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSCOnPreAttributeChange, UGSCAttributeSetBase*, AttributeSet, FGameplayAttribute, Attribute, float, NewValue);

/** GameplayEffect 실행 후 호출되는 이벤트 델리게이트: 변경된 속성, 소스/타겟 액터, 소스 태그, 추가 데이터 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGSCOnPostGameplayEffectExecute, FGameplayAttribute, Attribute, AActor*, SourceActor, AActor*, TargetActor, const FGameplayTagContainer&, SourceTags, const FGSCGameplayEffectExecuteData, Payload);

/** 능력이 활성화되었을 때 호출되는 이벤트 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGSCOnAbilityActivated, const UGameplayAbility*, Ability);

/** 능력이 종료되었을 때 호출되는 이벤트 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGSCOnAbilityEnded, const UGameplayAbility*, Ability);

/** 능력 활성 실패 시 호출되는 이벤트 델리게이트: 실패한 능력과 실패 사유 태그 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGSCOnAbilityFailed, const UGameplayAbility*, Ability, const FGameplayTagContainer&, ReasonTags);

/** GameplayEffect 스택 변경 시 호출되는 이벤트 델리게이트: 관련 태그, 효과 핸들, 새/이전 스택 개수 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGSCOnGameplayEffectStackChange, FGameplayTagContainer, AssetTags, FGameplayTagContainer, GrantedTags, FActiveGameplayEffectHandle, ActiveHandle, int32, NewStackCount, int32, OldStackCount);

/** GameplayEffect 추가 시 호출되는 이벤트 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSCOnGameplayEffectAdded, FGameplayTagContainer, AssetTags, FGameplayTagContainer, GrantedTags, FActiveGameplayEffectHandle, ActiveHandle);

/** GameplayEffect 제거 시 호출되는 이벤트 델리게이트 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSCOnGameplayEffectRemoved, FGameplayTagContainer, AssetTags, FGameplayTagContainer, GrantedTags, FActiveGameplayEffectHandle, ActiveHandle);

/** Gameplay 태그 스택 변경 시 호출되는 이벤트 델리게이트: 태그와 새로운 태그 카운트 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGSCOnGameplayTagStackChange, FGameplayTag, GameplayTag, int32, NewTagCount);

/** 능력 커밋 시 호출되는 이벤트 델리게이트 (비용/쿨다운 적용 시) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGSCOnAbilityCommit, UGameplayAbility*, Ability);

/** 쿨다운 변경 시 호출되는 이벤트 델리게이트: 능력, 쿨다운 태그, 남은 시간, 전체 지속 시간 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGSCOnCooldownChanged, UGameplayAbility*, Ability, const FGameplayTagContainer, CooldownTags, float, TimeRemaining, float, Duration);

/** 쿨다운 종료 시 호출되는 이벤트 델리게이트: 능력, 해당 쿨다운 태그, 지속 시간 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSCOnCooldownEnd, UGameplayAbility*, Ability, FGameplayTag, CooldownTag, float, Duration);

/** 데미지 처리 이벤트 델리게이트: 데미지 양, 소스 캐릭터, 데미지 태그 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FGSCOnDamage, float, DamageAmount, AActor*, SourceCharacter, const struct FGameplayTagContainer&, DamageTags);

/** GameplayEffect 시간 변경 시 호출되는 이벤트 델리게이트: 관련 태그, 효과 핸들, 새 시작 시간, 새 지속 시간 전달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FGSCOnGameplayEffectTimeChange, FGameplayTagContainer, AssetTags, FGameplayTagContainer, GrantedTags, FActiveGameplayEffectHandle, ActiveHandle, float, NewStartTime, float, NewDuration);

//================================================================================
// UGSCCoreComponent 클래스
// - 이 컴포넌트는 Ability System Component(ASC)에 대한 추상화를 제공하여, 
//   플레이어 상태(예: PlayerState) 등에서 ASC에 직접 접근할 수 없는 경우에도
//   공통 기능 및 이벤트를 제공하는 역할을 합니다.
// - 게임 캐릭터의 능력 부여, 속성 관리, 이벤트 전파 등 다양한 역할을 수행합니다.
//================================================================================
UCLASS(BlueprintType, Blueprintable, ClassGroup=("GASCompanion"), meta=(BlueprintSpawnableComponent))
class GASCOMPANION_API UGSCCoreComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//===========================================================================
	// 생성자 및 기본 초기화
	//===========================================================================

	/** 기본 생성자: 컴포넌트의 기본 속성값 설정 및 네트워크 복제를 기본적으로 활성화합니다. */
	UGSCCoreComponent();

protected:
	//===========================================================================
	// UActorComponent 인터페이스 재정의
	//===========================================================================

	/** 게임 시작 시 호출되는 함수 (BeginPlay override) */
	virtual void BeginPlay() override;

	/** 컴포넌트가 파괴되기 전에 호출되는 함수 (BeginDestroy override) */
	virtual void BeginDestroy() override;

public:
	//===========================================================================
	// 초기화 및 등록 관련 함수들
	//===========================================================================

	/**
	 * 소유 액터(Owner)를 초기화하고, 이를 통해 ASC 및 관련 참조를 설정합니다.
	 * 주로 BeginPlay()에서 호출되어 소유자 정보를 초기화합니다.
	 */
	void SetupOwner();

	/**
	 * ASC에 대한 델리게이트(이벤트 리스너)를 등록합니다.
	 * 이를 통해 속성 변화, GameplayEffect 추가/제거, 능력 활성화 등의 이벤트를 브루프린트로 전달합니다.
	 *
	 * @param ASC : 델리게이트를 등록할 Ability System Component
	 */
	void RegisterAbilitySystemDelegates(UAbilitySystemComponent* ASC);

	/**
	 * ASC에 등록된 델리게이트들을 해제합니다.
	 * 컴포넌트가 파괴될 때나 초기화 재실행 시에 호출되어 메모리 누수를 방지합니다.
	 * @param ASC : 델리게이트를 해제할 Ability System Component
	 */
	void ShutdownAbilitySystemDelegates(UAbilitySystemComponent* ASC);

	//===========================================================================
	// 게임플레이 관련 이벤트 핸들러 함수들 (속성 변화, 데미지 처리 등)
	//===========================================================================

	/**
	 * 데미지 처리를 위한 함수입니다.
	 * 데미지 이벤트를 브루프린트에 전달하며, 필요 시 추가 처리를 할 수 있습니다.
	 *
	 * @param DamageAmount : 적용된 데미지 양
	 * @param DamageTags : 데미지와 관련된 Gameplay 태그들
	 * @param SourceActor : 데미지를 발생시킨 액터 (예: 공격자)
	 */
	virtual void HandleDamage(float DamageAmount, const FGameplayTagContainer& DamageTags, AActor* SourceActor);

	/**
	 * 체력(Health) 변화 시 호출되는 함수입니다.
	 * 체력 변화 이벤트를 브루프린트로 전달하며, 체력이 0 이하일 경우 Die() 함수를 호출합니다.
	 * @param DeltaValue : 체력 변화량 (양수이면 회복, 음수이면 피해)
	 * @param EventTags : 체력 변화와 관련된 Gameplay 태그들
	 */
	virtual void HandleHealthChange(float DeltaValue, const FGameplayTagContainer& EventTags);

	/**
	 * 스태미나(Stamina) 변화 시 호출되는 함수입니다.
	 *
	 * @param DeltaValue : 스태미나 변화량
	 * @param EventTags : 스태미나 변화와 관련된 Gameplay 태그들
	 */
	virtual void HandleStaminaChange(float DeltaValue, const FGameplayTagContainer& EventTags);

	/**
	 * 마나(Mana) 변화 시 호출되는 함수입니다.
	 *
	 * @param DeltaValue : 마나 변화량
	 * @param EventTags : 마나 변화와 관련된 Gameplay 태그들
	 */
	virtual void HandleManaChange(float DeltaValue, const FGameplayTagContainer& EventTags);

	/**
	 * 특정 속성(Attribute)의 변화 이벤트를 처리하는 함수입니다.
	 *
	 * @param Attribute : 변화한 속성
	 * @param DeltaValue : 속성 변화량
	 * @param EventTags : 속성 변화와 관련된 Gameplay 태그들
	 */
	virtual void HandleAttributeChange(FGameplayAttribute Attribute, float DeltaValue, const FGameplayTagContainer& EventTags);


	//===========================================================================
	// 범용 속성 변화 콜백 함수들 (ASC 델리게이트에 의해 호출)
	//===========================================================================

	/** 
	 * 속성(BaseValue)이 변경될 때 호출되는 범용 콜백 함수.
	 * 이 함수는 속성의 이전 값과 새로운 값을 비교하여 변화가 있을 경우 브루프린트 이벤트를 브로드캐스트합니다.
	 * @param Data : 속성 변화에 대한 데이터를 포함하는 구조체
	 */
	virtual void OnAttributeChanged(const FOnAttributeChangeData& Data);

	/** 
	 * 데미지 관련 속성 변화 콜백 함수.
	 * 데미지 처리와 관련하여 추가적인 처리가 필요할 경우 이 함수를 오버라이드합니다.
	 *
	 * @param Data : 데미지 속성 변화에 대한 데이터를 포함하는 구조체
	 */
	virtual void OnDamageAttributeChanged(const FOnAttributeChangeData& Data);

	//===========================================================================
	// 사망 및 관련 함수
	//===========================================================================

	/**
	 * 캐릭터 사망 이벤트를 브로드캐스트합니다.
	 * 사망 처리(예: UI 업데이트, 게임 오버 처리 등)를 위해 브루프린트에서 이 이벤트를 수신할 수 있습니다.
	*/
	UPROPERTY(BlueprintAssignable, Category = "GAS Companion|Actor")
	FGSCOnDeath OnDeath;

	/**
	 * 캐릭터의 사망 처리를 실행합니다.
	 * 사망 시 호출되며, OnDeath 델리게이트를 통해 관련 이벤트를 브로드캐스트합니다.
	*/
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Actor")
	virtual void Die();

	//===========================================================================
	// 속성(Getter) 함수들
	//===========================================================================

	/** 현재 체력(Health) 값을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual float GetHealth() const;

	/** 최대 체력(MaxHealth) 값을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual float GetMaxHealth() const;

	/** 현재 스태미나(Stamina) 값을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual float GetStamina() const;

	/** 최대 스태미나(MaxStamina) 값을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual float GetMaxStamina() const;

	/** 현재 마나(Mana) 값을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual float GetMana() const;

	/** 최대 마나(MaxMana) 값을 반환합니다. */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual float GetMaxMana() const;

	/**
	 * 기본 속성 값(Base Value)을 반환합니다.
	 * 이는 상태 효과나 모디파이어가 적용되지 않은 원시 속성 값입니다.
	 *
	 * @param Attribute : 조회할 Gameplay 속성
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual float GetAttributeValue(FGameplayAttribute Attribute) const;

	/**
	 * @brief 특정 GameplayAttribute의 최종(Current) 값을 반환합니다.
	 *
	 * @param Attribute 조회할 속성
	 * @return 최종 계산된 값(모디파이어 포함)
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual float GetCurrentAttributeValue(FGameplayAttribute Attribute) const;

	/**
	 * 캐릭터가 살아있는지 여부를 반환합니다.
	 * 일반적으로 체력이 0보다 크면 살아있다고 판단합니다.
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual bool IsAlive() const;

	//===========================================================================
	// 능력(Ability) 관련 함수들
	//===========================================================================

	/**
	 * 주어진 능력을 캐릭터에게 부여하여 활성화 가능하도록 만듭니다.
	 * @param Ability : 부여할 Gameplay Ability 클래스
	 * @param Level : 능력의 레벨 (기본값은 1)
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GAS Companion|Abilities")
	virtual void GrantAbility(TSubclassOf<UGameplayAbility> Ability, int32 Level = 1);

	/**
	 * 주어진 능력을 캐릭터의 ASC에서 제거합니다.
	 * @param Ability : 제거할 Gameplay Ability 클래스
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GAS Companion|Abilities")
	virtual void ClearAbility(TSubclassOf<UGameplayAbility> Ability);

	/**
	 * 여러 능력을 한 번에 캐릭터의 ASC에서 제거합니다.
	*
	 * @param Abilities : 제거할 Gameplay Ability 클래스들의 배열
	*/
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GAS Companion|Abilities")
	virtual void ClearAbilities(TArray<TSubclassOf<UGameplayAbility>> Abilities);

	/**
	 * @brief ASC가 특정 태그를 가진 능력을 소유 중인지 확인합니다.
	 * @param TagContainer 확인할 GameplayTag 컨테이너
	 * @return 해당 태그를 가진 능력이 하나 이상 있으면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|GameplayTags")
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer TagContainer) const;

	/**
	 * @brief ASC가 특정 단일 태그를 가진 능력을 소유 중인지 확인합니다.
	 * @param TagToCheck 확인할 단일 GameplayTag
	 * @return 해당 태그를 가진 능력이 있으면 true
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|GameplayTags")
	virtual bool HasMatchingGameplayTag(const FGameplayTag TagToCheck) const;

	/**
	 * @brief 특정 Ability 클래스를 사용 중인지(활성화된 인스턴스가 있는지) 확인합니다.
	 * @param AbilityClass 확인할 Ability 클래스
	 * @return 사용 중이면 true
	 */
	UFUNCTION(BlueprintPure, Category="GAS Companion|Abilities")
	bool IsUsingAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass);

	/**
	 * @brief 특정 Ability 태그를 가진 능력이 실행 중인지 확인합니다.
	 * @param AbilityTags 확인할 태그들
	 * @return 실행 중인 능력이 있으면 true
	 */
	UFUNCTION(BlueprintPure, Category="GAS Companion|Abilities")
	bool IsUsingAbilityByTags(FGameplayTagContainer AbilityTags);

	/**
	 * @brief 특정 Ability 클래스를 가진 활성 능력 인스턴스 목록을 반환합니다.
	 * @param AbilityToSearch 검색할 Ability 클래스
	 * @return 활성화된 Ability 인스턴스 목록
	 */
	UFUNCTION(BlueprintCallable, Category="GAS Companion|Abilities")
	TArray<UGameplayAbility*> GetActiveAbilitiesByClass(TSubclassOf<UGameplayAbility> AbilityToSearch) const;

	/**
	 * @brief 특정 태그와 매칭되는 활성 능력 인스턴스 목록을 반환합니다.
	 *
	 * @param GameplayTagContainer 확인할 태그들
	 * @return 실행 중인 능력 인스턴스 목록
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	virtual TArray<UGameplayAbility*> GetActiveAbilitiesByTags(const FGameplayTagContainer GameplayTagContainer) const;

	/**
	 * @brief 특정 Ability 클래스를 활성화 시도합니다.
	 *
	 * @param AbilityClass 활성화할 Ability 클래스
	 * @param ActivatedAbility 활성화에 성공한 경우 할당될 UGSCGameplayAbility 인스턴스
	 * @param bAllowRemoteActivation 원격(서버/클라) 활성화 허용 여부
	 * @return 활성화 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	virtual bool ActivateAbilityByClass(TSubclassOf<UGameplayAbility> AbilityClass, UGSCGameplayAbility*& ActivatedAbility, bool bAllowRemoteActivation = true);

	/**
	 * @brief 특정 태그와 매칭되는 단일 Ability를 활성화 시도합니다.
	 *
	 * @param AbilityTags 활성화할 Ability 태그
	 * @param ActivatedAbility 활성화 성공 시 할당될 Ability 인스턴스
	 * @param bAllowRemoteActivation 원격(서버/클라) 활성화 허용 여부
	 * @return 활성화 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Abilities")
	virtual bool ActivateAbilityByTags(const FGameplayTagContainer AbilityTags, UGSCGameplayAbility*& ActivatedAbility, const bool bAllowRemoteActivation = true);

	//===========================================================================
	// 속성 설정 및 조정 함수들
	//===========================================================================

	/**
	 * 특정 속성의 기본 값을 설정합니다.
	 * 현재 활성화된 모디파이어에는 영향을 주지 않습니다.
	 *
	 * @param Attribute : 설정할 Gameplay 속성
	 * @param NewValue : 새로 적용할 기본 값
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual void SetAttributeValue(FGameplayAttribute Attribute, float NewValue);

	/**
	 * 특정 속성 값을 주어진 최소/최대 범위로 클램프(제한)합니다.
	 * @param Attribute : 클램프할 Gameplay 속성
	 * @param MinValue : 최소 허용 값
	 * @param MaxValue : 최대 허용 값
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual void ClampAttributeValue(FGameplayAttribute Attribute, float MinValue, float MaxValue);

	/**
	 * 최대 속성값(MaxAttribute)이 변경될 때, 관련 속성(AffectedAttribute)의 값을 비례적으로 조정합니다.
	 * 예를 들어, 최대 체력이 증가하면 현재 체력도 동일 비율로 증가시킵니다.
	*
	 * @param AttributeSet : 속성들이 소속된 AttributeSet
	 * @param AffectedAttributeProperty : 조정할 속성
	 * @param MaxAttribute : 관련된 최대 속성
	 * @param NewMaxValue : 새로 적용될 최대 속성 값
	*/
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Attributes")
	virtual void AdjustAttributeForMaxChange(UPARAM(ref) UGSCAttributeSetBase* AttributeSet, const FGameplayAttribute AffectedAttributeProperty, const FGameplayAttribute MaxAttribute, float NewMaxValue);

	//===========================================================================
	// AttributeSet 이벤트 관련 함수들
	//===========================================================================

	/**
	 * 속성 변경 전 이벤트 처리 함수.
	 * AttributeSet에서 속성 값이 변경되기 전에 호출되어, 브루프린트 이벤트로 전달합니다.
	 *
	 * @param AttributeSet : 변경을 발생시킨 AttributeSet
	 * @param Attribute : 변경될 속성
	 * @param NewValue : 새로 적용될 값
	 */
	virtual void PreAttributeChange(UGSCAttributeSetBase* AttributeSet, const FGameplayAttribute& Attribute, float NewValue);

	/**
	 * GameplayEffect 실행 후 호출되는 이벤트 처리 함수.
	 * GameplayEffect에 의해 속성 값이 변경된 후, 추가적인 처리가 필요한 경우 호출됩니다.
	 *
	 * @param AttributeSet : 변경을 발생시킨 AttributeSet
	 * @param Data : GameplayEffect 모디파이어 관련 데이터 (실제 변화량, 컨텍스트 등)
	 */
	virtual void PostGameplayEffectExecute(UGSCAttributeSetBase* AttributeSet, const FGameplayEffectModCallbackData& Data);

	/**
	 * ASC의 능력들이 부여되었거나 제거되었음을 알리는 이벤트.
	 * bStartupAbilitiesGranted 플래그를 통해 초기 능력 부여 상태를 관리합니다.
	 *
	 * @param bGranted : 초기 능력 부여 여부
	 */
	void SetStartupAbilitiesGranted(bool bGranted);


	//===========================================================================
	// 내부 구현용 변수 및 함수들
	//===========================================================================
protected:
	/**
	 * @brief GameplayEffect가 ASC에 추가되었을 때 호출되는 내부 함수입니다.
	 *
	 * @param Target Effect가 적용된 대상 ASC
	 * @param SpecApplied 적용된 EffectSpec
	 * @param ActiveHandle 생성된 FActiveGameplayEffectHandle
	 */
	virtual void OnActiveGameplayEffectAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, const FActiveGameplayEffectHandle ActiveHandle);

	/**
	 * @brief GameplayEffect 스택 개수가 변경될 때 호출되는 내부 함수입니다.
	 * @param ActiveHandle 변경된 GameplayEffect의 핸들
	 * @param NewStackCount 새로운 스택 개수
	 * @param PreviousStackCount 이전 스택 개수
	 */
	virtual void OnActiveGameplayEffectStackChanged(FActiveGameplayEffectHandle ActiveHandle, int32 NewStackCount, int32 PreviousStackCount);

	/**
	 * @brief GameplayEffect 시간(지속 시간 등)이 변경될 때 호출되는 내부 함수입니다.
	 * @param ActiveHandle 변경된 GameplayEffect의 핸들
	 * @param NewStartTime 새 시작 시간
	 * @param NewDuration 새 지속 시간
	 */
	virtual void OnActiveGameplayEffectTimeChanged(FActiveGameplayEffectHandle ActiveHandle, float NewStartTime, float NewDuration);

	/**
	 * @brief GameplayEffect가 제거될 때 호출되는 내부 함수입니다.
	 * @param EffectRemoved 제거된 FActiveGameplayEffect 구조체
	 */
	virtual void OnAnyGameplayEffectRemoved(const FActiveGameplayEffect& EffectRemoved);

	/**
	 * @brief 새로운 GameplayTag가 추가되거나 제거될 때 호출되는 내부 함수입니다.
	 * @param GameplayTag 변경된 태그
	 * @param NewCount 새로운 카운트(0이면 제거, 1 이상이면 추가)
	 */
	void OnAnyGameplayTagChanged(FGameplayTag GameplayTag, int32 NewCount) const;

	/**
	 * @brief 능력이 커밋(비용/쿨다운 적용 등)될 때 호출되는 내부 함수입니다.
	 * @param ActivatedAbility 커밋된 Ability
	 */
	void OnAbilityCommitted(UGameplayAbility* ActivatedAbility);

	/**
	 * @brief 쿨다운 관련 GameplayTag가 변경될 때 호출되는 내부 함수입니다.
	 * @param GameplayTag 변경된 태그
	 * @param NewCount 새로운 태그 카운트(0이면 제거)
	 * @param AbilitySpecHandle 해당 AbilitySpec 핸들
	 * @param Duration 쿨다운 전체 지속 시간
	 */
	virtual void OnCooldownGameplayTagChanged(const FGameplayTag GameplayTag, const int32 NewCount, FGameplayAbilitySpecHandle AbilitySpecHandle, float Duration);

	/**
	 * @brief 능력 커밋 시 쿨다운 처리를 위한 내부 헬퍼 함수입니다.
	 *
	 * @param ActivatedAbility 커밋된 Ability
	 */
	void HandleCooldownOnAbilityCommit(UGameplayAbility* ActivatedAbility);


	//===========================================================================
	// 멤버 변수: 소유자 및 Ability System 관련 변수들
	//===========================================================================
public:
	/** 이 컴포넌트를 소유하는 액터. 게임플레이 내에서 캐릭터 또는 다른 액터를 나타냅니다. */
	UPROPERTY()
	TObjectPtr<AActor> OwnerActor;

	/** 소유 액터가 Pawn인 경우 Pawn으로 캐스팅한 값 */
	UPROPERTY()
	TObjectPtr<APawn> OwnerPawn;

	/** 소유 액터가 Character인 경우 Character로 캐스팅한 값 */
	UPROPERTY()
	TObjectPtr<ACharacter> OwnerCharacter;

	/** 소유 액터에 연결된 Ability System Component (ASC). 능력 및 속성 관리에 사용됩니다. */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> OwnerAbilitySystemComponent;

protected:
	/** 초기 능력 부여 여부를 관리하는 내부 플래그 */
	bool bStartupAbilitiesGranted = false;

private:
	/** 
	 * 델리게이트 바인딩을 해제하기 위해, 등록된 활성 GameplayEffect 핸들을 저장하는 배열.
	 */
	TArray<FActiveGameplayEffectHandle> GameplayEffectAddedHandles;

	/** 
	 * 델리게이트 바인딩을 해제하기 위해, 등록된 Gameplay 태그들을 저장하는 배열.
	 */
	TArray<FGameplayTag> GameplayTagBoundToDelegates;

public:
	/**
	 * ASC의 InitAbilityActorInfo 호출 직후, 능력과 속성이 부여된 후에 실행되는 이벤트입니다.
	 *
	 * 이 이벤트는 클라이언트와 서버 모두에서 여러 번 발생할 수 있습니다:
	 *
	 * - 서버: 컴포넌트 초기화 후 한 번 발생
	 * - 서버: 소유 액터의 복제(예: Player State에 의해 소유된 경우) 후 한 번 발생
	 * - 클라이언트: 컴포넌트 초기화 후 한 번 발생
	 * - 클라이언트: 소유 액터의 복제(예: Player State OnRep_PlayerState) 후 한 번 더 발생
	 *
	 * 또한 ASC가 Pawn에 위치하는지, Player State에 위치하는지에 따라 발생 시점이 달라집니다.
	 *
	 * 참고: 이 이벤트는 GSCAbilitySystemComponent에도 노출되어 있어, PlayerState에 ASC를 사용하는 Pawn에서도
	 * 해당 이벤트에 반응할 수 있습니다.
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Abilities")
	FGSCOnInitAbilityActorInfoCore OnInitAbilityActorInfo;

	/**
	 * 캐릭터가 데미지를 입었을 때 호출되는 이벤트.
	*
	 * @param DamageAmount : 적용된 데미지 양 (클램핑 이전의 원시값)
	 * @param SourceCharacter : 데미지를 준 액터 (예: 공격자)
	 * @param DamageTags : 데미지와 관련된 Gameplay 태그들
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Abilities")
	FGSCOnDamage OnDamage;

	/**
	 * 체력 변화 이벤트: 데미지 또는 회복에 의한 체력 변화 시 호출됩니다.
	*
	 * @param DeltaValue : 체력 변화량 (양수면 회복, 음수면 피해)
	 * @param EventTags : 체력 변화와 관련된 Gameplay 태그들
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Abilities")
	FGSCOnDefaultAttributeChange OnHealthChange;

	/**
	 * 스태미나 변화 이벤트: 스태미나 회복 또는 사용 시 호출됩니다.
	*
	 * @param DeltaValue : 스태미나 변화량
	 * @param EventTags : 스태미나 변화와 관련된 Gameplay 태그들
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Abilities")
	FGSCOnDefaultAttributeChange OnStaminaChange;

	/**
	 * 마나 변화 이벤트: 마나 회복 또는 사용 시 호출됩니다.
	*
	 * @param DeltaValue : 마나 변화량
	 * @param EventTags : 마나 변화와 관련된 Gameplay 태그들
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Abilities")
	FGSCOnDefaultAttributeChange OnManaChange;

	/**
	 * 모든 속성(Attribute) 변화 이벤트: 특정 속성이 변경될 때 호출됩니다.
	*
	 * @param Attribute : 변경된 속성
	 * @param DeltaValue : 변화량 (추가 연산 결과 또는 데미지량)
	 * @param EventTags : 속성 변화와 관련된 Gameplay 태그들
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Abilities")
	FGSCOnAttributeChange OnAttributeChange;


	//===========================================================================
	// 능력 이벤트 (Ability Events)
	//===========================================================================
public:
	/**
	 * 능력 활성화 시 호출되는 이벤트.
	 * 능력이 성공적으로 활성화되었을 때 브루프린트에서 처리할 수 있도록 전달합니다.
	*/
	UPROPERTY(BlueprintAssignable, Category = "GAS Companion|Ability")
	FGSCOnAbilityActivated OnAbilityActivated;

	/**
	 * 능력이 종료될 때 호출되는 이벤트.
	*/
	UPROPERTY(BlueprintAssignable, Category = "GAS Companion|Ability")
	FGSCOnAbilityEnded OnAbilityEnded;

	/**
	 * 능력 활성에 실패했을 때 호출되는 이벤트.
	 * 실패한 능력과 그 이유(태그)를 전달합니다.
	*/
	UPROPERTY(BlueprintAssignable, Category = "GAS Companion|Ability")
	FGSCOnAbilityFailed OnAbilityFailed;

	/**
	 * GameplayEffect가 추가되거나 제거될 때 호출되는 이벤트.
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Ability")
	FGSCOnGameplayEffectStackChange OnGameplayEffectStackChange;

	/**
	 * GameplayEffect의 지속 시간이 변경될 때 호출되는 이벤트 (예: 지속 시간 갱신).
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Ability")
	FGSCOnGameplayEffectTimeChange OnGameplayEffectTimeChange;

	/**
	 * GameplayEffect가 추가될 때 호출되는 이벤트.
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Ability")
	FGSCOnGameplayEffectAdded OnGameplayEffectAdded;

	/**
	 * GameplayEffect가 제거될 때 호출되는 이벤트.
	*/
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Ability")
	FGSCOnGameplayEffectRemoved OnGameplayEffectRemoved;

	/**
	 * 새로운 Gameplay 태그가 추가되거나 제거될 때 호출되는 이벤트.
	 * 단, 단순히 태그 카운트가 증가하는 경우는 제외합니다.
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Ability")
	FGSCOnGameplayTagStackChange OnGameplayTagChange;

	/**
	 * 능력 커밋(비용/쿨다운 적용) 시 호출되는 이벤트.
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Ability")
	FGSCOnAbilityCommit OnAbilityCommit;

	/**
	 * 쿨다운이 시작될 때(능력 커밋 후) 호출되는 이벤트.
	 * 남은 시간 및 전체 지속 시간 정보를 함께 전달합니다.
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Ability")
	FGSCOnCooldownChanged OnCooldownStart;

	/**
	 * 쿨다운이 종료(만료)될 때 호출되는 이벤트.
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Ability")
	FGSCOnCooldownEnd OnCooldownEnd;

public:
	/**
	 * GameplayEffect 실행 후, 블루프린트에 전달되는 이벤트.
	 * 속성 값 변경, 소스 및 타겟 액터, 소스 태그, 그리고 추가 데이터(Payload)를 전달합니다.
	*/
	UPROPERTY(BlueprintAssignable, Category = "GAS Companion|Attributes")
	FGSCOnPostGameplayEffectExecute OnPostGameplayEffectExecute;

	/**
	 * 속성 값이 변경되기 직전에 호출되는 이벤트.
	 * 변경 전의 상태를 브루프린트에서 확인할 수 있도록 전달합니다.
	*/
	UPROPERTY(BlueprintAssignable, Category = "GAS Companion|Attributes")
	FGSCOnPreAttributeChange OnPreAttributeChange;
};
