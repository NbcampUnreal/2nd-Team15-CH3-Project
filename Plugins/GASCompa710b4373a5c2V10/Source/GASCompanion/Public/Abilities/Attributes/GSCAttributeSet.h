// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Attributes/GSCAttributeSetBase.h"
#include "GSCAttributeSet.generated.h"

/**
 * 대부분의 게임에서 사용되는 기본 속성들(체력, 스태미나, 마나)을 포함합니다.
 * 피해를 받거나 마나, 스태미나 같은 자원을 사용하는 캐릭터는 이 AttributeSet을 사용합니다.
 *
 * 속성 목록:
 * - Health: 캐릭터가 현재 보유한 체력.
 * - MaxHealth: 캐릭터가 가질 수 있는 최대 체력.
 * - HealthRegenRate: Gameplay Effect 기간마다 회복되는 체력의 양을 결정하는 백업 속성.
 *
 * - Stamina: 주로 능력 실행에 사용되는 자원.
 * - MaxStamina: 캐릭터가 가질 수 있는 최대 스태미나.
 * - StaminaRegenRate: Gameplay Effect를 통해 사용될 때 회복되는 스태미나의 양을 결정하는 백업 속성.
 *
 * - Mana: 주로 특수 능력 실행에 사용되는 자원.
 * - MaxMana: 캐릭터가 가질 수 있는 최대 마나.
 * - ManaRegenRate: 마나 회복량을 결정하는 백업 속성.
 *
 * - Damage: DamageExecution 또는 Gameplay Effect에서 최종 피해를 계산하는 데 사용되는 메타 속성이며,계산된 피해는 체력(Health)의 감소로 이어집니다.
 * - StaminaDamage: DamageExecution 또는 Gameplay Effect에서 최종 피해를 계산하는 데 사용되며, 계산된 피해는 스태미나(Stamina)의 감소로 이어집니다.
 */
UCLASS()
class GASCOMPANION_API UGSCAttributeSet : public UGSCAttributeSetBase
{
	GENERATED_BODY()

public:
	//----------------------------------------
	// 오버라이드 함수 (부모 클래스 기능 확장)
	//----------------------------------------
	/**
	 * @brief 속성 변경 전 사전 처리 함수 (PreAttributeChange)
	 *
	 * 속성이 변경되기 전에 호출되며, 특히 최대 체력, 스태미나, 마나가 변경될 때
	 * 현재 값(예: Health, Stamina, Mana)을 비례적으로 조정하여 캐릭터의 상대적 상태를 유지합니다.
	 *
	 * @param Attribute 변경될 속성 (예: MaxHealth)
	 * @param NewValue 변경 후 적용될 새 값 (아직 반영되기 전이며, 여기서 수정 가능)
	 */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	/**
	 * @brief GameplayEffect 적용 후 후처리 함수 (PostGameplayEffectExecute)
	 *
	 * GameplayEffect가 적용되어 실제 속성 값이 변경된 후 호출되며,
	 * Damage, Health, Stamina, Mana 등 각 속성에 따른 추가 처리를 실행합니다.
	 *
	 * @param Data GameplayEffect 적용에 대한 상세 정보를 포함하는 FGameplayEffectModCallbackData
	 */
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/**
	 * @brief 네트워크 복제를 위해 복제 대상 속성들을 등록하는 함수
	 *
	 * 언리얼 엔진의 네트워크 복제 시스템에 의해 호출되며, 이 클래스의 속성들 중
	 * 복제할 속성들을 OutLifetimeProps 배열에 추가합니다.
	 *
	 * @param OutLifetimeProps 네트워크 복제 대상 속성 목록이 저장될 배열
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//----------------------------------------
	// 기본 속성 변수 및 관련 매크로
	//----------------------------------------
	/**
	 * @brief 현재 체력(Health)
	 *
	 * - 캐릭터가 현재 보유한 체력을 나타냅니다.
	 * - 체력이 0 이하가 되면 캐릭터는 사망할 것으로 예상되며,
	 *   음수로 직접 변경되지 않고, Damage 메타 속성을 통해 Health 감소가 처리됩니다.
	 * - 값은 MaxHealth에 의해 상한이 정해지며, 네트워크를 통해 복제됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_Health)
	FGameplayAttributeData Health = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, Health)

	/**
	 * @brief 최대 체력(MaxHealth)
	 *
	 * - 캐릭터가 가질 수 있는 최대 체력을 나타냅니다.
	 * - GameplayEffect에 의해 변경될 수 있으므로 독립된 속성으로 관리됩니다.
	 * - 네트워크 복제 시 OnRep_MaxHealth 함수를 통해 최신 상태를 반영합니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, MaxHealth)

	/**
	 * @brief 체력 재생 속도 (HealthRegenRate)
	 *
	 * - 일정 주기마다 체력을 회복하는 양을 결정하는 백업 속성입니다.
	 * - GameplayEffect에 의해 변경될 수 있으며, 네트워크 복제됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_HealthRegenRate)
	FGameplayAttributeData HealthRegenRate = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, HealthRegenRate)

	/**
	 * @brief 현재 스태미나 (Stamina)
	 *
	 * - 캐릭터가 능력 실행 시 사용하는 스태미나 자원입니다.
	 * - MaxStamina에 의해 제한되며, 네트워크 복제됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_Stamina)
	FGameplayAttributeData Stamina = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, Stamina)

	/**
	 * @brief 최대 스태미나 (MaxStamina)
	 *
	 * - 캐릭터가 가질 수 있는 최대 스태미나 양을 나타냅니다.
	 * - 별도의 속성으로 관리되어 GameplayEffect에 의해 변경될 수 있으며,
	 *   네트워크 복제 시 OnRep_MaxStamina를 통해 최신 값이 반영됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_MaxStamina)
	FGameplayAttributeData MaxStamina = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, MaxStamina)

	/**
	 * @brief 스태미나 재생 속도 (StaminaRegenRate)
	 *
	 * - 일정 주기마다 스태미나를 회복하는 양을 결정하는 속성입니다.
	 * - 네트워크 복제 시 OnRep_StaminaRegenRate 함수를 통해 업데이트 됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_StaminaRegenRate)
	FGameplayAttributeData StaminaRegenRate = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, StaminaRegenRate)

	/**
	 * @brief 현재 마나 (Mana)
	 *
	 * - 캐릭터가 특수 능력 사용 시 소비하는 마나 자원입니다.
	 * - MaxMana에 의해 제한되며, 네트워크 복제 됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_Mana)
	FGameplayAttributeData Mana = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, Mana)

	/**
	 * @brief 최대 마나 (MaxMana)
	 *
	 * - 캐릭터가 가질 수 있는 최대 마나 양을 나타냅니다.
	 * - GameplayEffect에 의해 변경될 수 있으며, 네트워크 복제 시 OnRep_MaxMana를 통해 최신 상태가 반영됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_MaxMana)
	FGameplayAttributeData MaxMana = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, MaxMana)

	/**
	 * @brief 마나 재생 속도 (ManaRegenRate)
	 *
	 * - 일정 주기마다 마나를 회복하는 양을 결정하는 속성입니다.
	 * - 네트워크 복제 시 OnRep_ManaRegenRate를 통해 최신 값이 반영됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", ReplicatedUsing = OnRep_ManaRegenRate)
	FGameplayAttributeData ManaRegenRate = 0.0f;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, ManaRegenRate)

	/**
	 * @brief Damage 메타 속성
	 *
	 * - DamageExecution 또는 GameplayEffect에서 최종 피해 계산에 사용됩니다.
	 * - 계산 후 Health에 적용되어 체력을 감소시킵니다.
	 * - 이 속성은 서버에서만 임시로 사용되며, 복제되지 않습니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", meta = (HideFromLevelInfos))
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, Damage)

	/**
	 * @brief StaminaDamage 메타 속성
	 *
	 * - DamageExecution 또는 GameplayEffect에서 스태미나 감소량 계산에 사용되며,
	 *   계산 후 Stamina에 적용됩니다.
	 * - 이 속성은 서버에서만 임시로 사용되며, 복제되지 않습니다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Attribute Set", meta = (HideFromLevelInfos))
	FGameplayAttributeData StaminaDamage;
	ATTRIBUTE_ACCESSORS(UGSCAttributeSet, StaminaDamage)

protected:
	//----------------------------------------
	// 복제 알림 함수 (Replication Notify Functions)
	//----------------------------------------
	/**
	 * @brief Health 속성 복제 시 호출되는 함수.
	 *
	 * 이 함수는 Health 값이 변경되어 네트워크로 복제된 후 호출되며,
	 * GAMEPLAYATTRIBUTE_REPNOTIFY 매크로를 통해 블루프린트에 변경 사항을 전달합니다.
	 *
	 * @param OldHealth 이전 Health 값
	 */
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldHealth);

	/**
	 * @brief MaxHealth 속성 복제 시 호출되는 함수.
	 *
	 * @param OldMaxHealth 이전 MaxHealth 값
	 */
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);

	/**
	 * @brief HealthRegenRate 속성 복제 시 호출되는 함수.
	 *
	 * @param OldHealthRegenRate 이전 HealthRegenRate 값
	 */
	UFUNCTION()
	virtual void OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate);

	/**
	 * @brief Mana 속성 복제 시 호출되는 함수.
	 *
	 * @param OldMana 이전 Mana 값
	 */
	UFUNCTION()
	virtual void OnRep_Mana(const FGameplayAttributeData& OldMana);

	/**
	 * @brief MaxMana 속성 복제 시 호출되는 함수.
	 *
	 * @param OldMaxMana 이전 MaxMana 값
	 */
	UFUNCTION()
	virtual void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana);

	/**
	 * @brief ManaRegenRate 속성 복제 시 호출되는 함수.
	 *
	 * @param OldManaRegenRate 이전 ManaRegenRate 값
	 */
	UFUNCTION()
	virtual void OnRep_ManaRegenRate(const FGameplayAttributeData& OldManaRegenRate);

	/**
	 * @brief Stamina 속성 복제 시 호출되는 함수.
	 *
	 * @param OldStamina 이전 Stamina 값
	 */
	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldStamina);

	/**
	 * @brief MaxStamina 속성 복제 시 호출되는 함수.
	 *
	 * @param OldMaxStamina 이전 MaxStamina 값
	 */
	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);

	/**
	 * @brief StaminaRegenRate 속성 복제 시 호출되는 함수.
	 *
	 * @param OldStaminaRegenRate 이전 StaminaRegenRate 값
	 */
	UFUNCTION()
	virtual void OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate);

	//----------------------------------------
	// 속성 값 클램프 및 내부 처리 함수
	//----------------------------------------
	/**
	 * @brief 속성 값을 최소값과 최대값 사이로 클램프하여 설정합니다.
	 *
	 * 네트워크 ASC(Ability System Component)를 통해 속성의 기본 값을 설정하며, 최소값은 GetClampMinimumValueFor()로 조회한 값을 사용합니다.
	 *
	 * @param Attribute 클램프할 속성
	 * @param Value 적용할 새 값
	 * @param MaxValue 속성의 최대 허용 값
	 */
	virtual void SetAttributeClamped(const FGameplayAttribute& Attribute, const float Value, const float MaxValue);

	/**
	 * @brief Damage 메타 속성이 적용된 후의 후처리를 담당합니다.
	 *
	 * DamageExecution에 의해 계산된 Damage 값을 Health 감소로 적용하며, 대상 Core 컴포넌트를 통해 피해 이벤트를 추가로 전달합니다.
	 *
	 * @param ExecutionData 효과 실행에 관련된 다양한 정보를 담은 구조체
	 */
	virtual void HandleDamageAttribute(const FGSCAttributeSetExecutionData& ExecutionData);

	/**
	 * @brief StaminaDamage 메타 속성이 적용된 후의 후처리를 담당합니다.
	 *
	 * 계산된 스태미나 피해(StaminaDamage)를 Stamina 감소로 적용하며, 대상 Core 컴포넌트를 통해 스태미나 변화 이벤트를 전달합니다.
	 *
	 * @param ExecutionData 효과 실행 관련 정보를 담은 구조체
	 */
	virtual void HandleStaminaDamageAttribute(const FGSCAttributeSetExecutionData& ExecutionData);

	/**
	 * @brief Health 속성이 적용된 후의 후처리 함수.
	 *
	 * 현재 Health 값을 클램프한 후, 대상 Core 컴포넌트를 통해 체력 변화 이벤트를 전달합니다.
	 *
	 * @param ExecutionData 효과 실행 관련 정보를 담은 구조체
	 */
	virtual void HandleHealthAttribute(const FGSCAttributeSetExecutionData& ExecutionData);

	/**
	 * @brief Stamina 속성이 적용된 후의 후처리 함수.
	 *
	 * 현재 Stamina 값을 클램프한 후, 대상 Core 컴포넌트를 통해 스태미나 변화 이벤트를 전달합니다.
	 *
	 * @param ExecutionData 효과 실행 관련 정보를 담은 구조체
	 */
	virtual void HandleStaminaAttribute(const FGSCAttributeSetExecutionData& ExecutionData);

	/**
	 * @brief Mana 속성이 적용된 후의 후처리 함수.
	 *
	 * 현재 Mana 값을 클램프한 후, 대상 Core 컴포넌트를 통해 마나 변화 이벤트를 전달합니다.
	 *
	 * @param ExecutionData 효과 실행 관련 정보를 담은 구조체
	 */
	virtual void HandleManaAttribute(const FGSCAttributeSetExecutionData& ExecutionData);
};
