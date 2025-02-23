
#include "Abilities/Attributes/GSCAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Components/GSCCoreComponent.h"
#include "Net/UnrealNetwork.h"
#include "GSCLog.h"


void UGSCAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    // 부모 클래스의 기본 PreAttributeChange 처리 호출 (필요한 기본 동작 수행)
    Super::PreAttributeChange(Attribute, NewValue);

    // 최대 체력, 스태미나, 마나가 변경될 때, 해당 속성의 현재 값을 비례적으로 조정합니다.
	if (Attribute == GetMaxHealthAttribute())
	{
        // 현재 Health 값을 MaxHealth의 변경 비율에 맞춰 조정합니다.
		AdjustAttributeForMaxChange(Health, MaxHealth, NewValue, GetHealthAttribute());
		return;
	}

	if (Attribute == GetMaxStaminaAttribute())
	{
		AdjustAttributeForMaxChange(Stamina, MaxStamina, NewValue, GetStaminaAttribute());
		return;
	}

	if (Attribute == GetMaxManaAttribute())
	{
		AdjustAttributeForMaxChange(Mana, MaxMana, NewValue, GetManaAttribute());
		return;
	}
}

void UGSCAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    // 부모 클래스의 기본 후처리 호출
    Super::PostGameplayEffectExecute(Data);

    // 매우 상세한 로그 출력 (디버깅용)
    GSC_WLOG(VeryVerbose, TEXT("PostGameplayEffectExecute called for %s.%s"), *GetName(), *Data.EvaluatedData.Attribute.AttributeName);

    // FGSCAttributeSetExecutionData 구조체에 효과 적용에 필요한 정보를 추출합니다.
	FGSCAttributeSetExecutionData ExecutionData;
	GetExecutionDataFromMod(Data, ExecutionData);

    // 평가된 속성에 따라 각각의 처리 함수를 호출합니다.
    if (Data.EvaluatedData.Attribute == GetDamageAttribute())
    {
        // Damage 메타 속성 처리 (Health 감소로 이어짐)
    	HandleDamageAttribute(ExecutionData);
    }
	else if (Data.EvaluatedData.Attribute == GetStaminaDamageAttribute())
	{
        // StaminaDamage 메타 속성 처리 (Stamina 감소로 이어짐)
		HandleStaminaDamageAttribute(ExecutionData);
	}
    else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // Health 속성의 후처리: 체력 클램프 및 이벤트 전달 처리
    	HandleHealthAttribute(ExecutionData);
    }
    else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
    {
        // Stamina 속성의 후처리: 스태미나 클램프 및 이벤트 전달 처리
    	HandleStaminaAttribute(ExecutionData);
    }
    else if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        // Mana 속성의 후처리: 마나 클램프 및 이벤트 전달 처리
    	HandleManaAttribute(ExecutionData);
    }
}

void UGSCAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    // 부모 클래스의 복제 설정 호출
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 아래 매크로를 통해 각 속성을 복제 대상 및 복제 알림 조건과 함께 등록합니다.
    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, HealthRegenRate, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, MaxStamina, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, StaminaRegenRate, COND_None, REPNOTIFY_Always);

    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UGSCAttributeSet, ManaRegenRate, COND_None, REPNOTIFY_Always);
}

void UGSCAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, Health, OldHealth);
}

void UGSCAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, MaxHealth, OldMaxHealth);
}

void UGSCAttributeSet::OnRep_HealthRegenRate(const FGameplayAttributeData& OldHealthRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, HealthRegenRate, OldHealthRegenRate);
}

void UGSCAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, Mana, OldMana);
}

void UGSCAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, MaxMana, OldMaxMana);
}

void UGSCAttributeSet::OnRep_ManaRegenRate(const FGameplayAttributeData& OldManaRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, ManaRegenRate, OldManaRegenRate);
}

void UGSCAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, Stamina, OldStamina);
}

void UGSCAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, MaxStamina, OldMaxStamina);
}

void UGSCAttributeSet::OnRep_StaminaRegenRate(const FGameplayAttributeData& OldStaminaRegenRate)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UGSCAttributeSet, StaminaRegenRate, OldStaminaRegenRate);
}

void UGSCAttributeSet::SetAttributeClamped(const FGameplayAttribute& Attribute, const float Value, const float MaxValue)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		// ASC가 유효하지 않으면 처리를 중단합니다.
		return;
	}

	// 최소 클램프 값을 조회한 후, Value를 최소값과 MaxValue 사이로 제한합니다.
	const float Min = GetClampMinimumValueFor(Attribute);
	const float NewValue = FMath::Clamp(Value, Min, MaxValue);

	// ASC를 통해 속성의 기본 값을 업데이트합니다.
	ASC->SetNumericAttributeBase(Attribute, NewValue);
}

void UGSCAttributeSet::HandleDamageAttribute(const FGSCAttributeSetExecutionData& ExecutionData)
{
	AActor* SourceActor = ExecutionData.SourceActor;
	AActor* TargetActor = ExecutionData.TargetActor;
	UGSCCoreComponent* TargetCoreComponent = ExecutionData.TargetCoreComponent;
	const FGameplayTagContainer SourceTags = ExecutionData.SourceTags;

	// Damage 속성의 현재 값을 로컬 변수에 저장한 후, Damage 속성을 0으로 초기화합니다.
	const float LocalDamageDone = GetDamage();
	SetDamage(0.f);

	// 만약 계산된 피해가 0보다 크다면 처리합니다.
	if (LocalDamageDone > 0.f)
	{
		// 먼저, 대상이 아직 살아있는지 확인합니다.
		bool bAlive = true;
		if (TargetCoreComponent)
		{
			bAlive = TargetCoreComponent->IsAlive();
			if (!bAlive)
			{
				GSC_LOG(Warning, TEXT("UGSCAttributeSet::PostGameplayEffectExecute() %s character or pawn is NOT alive when receiving damage"), *TargetActor->GetName());
			}
		}

		// 대상이 살아있다면 체력을 감소시키고, 클램프 처리를 적용합니다.
		if (bAlive)
		{
			const float NewHealth = GetHealth() - LocalDamageDone;
			const float ClampMinimumValue = GetClampMinimumValueFor(GetHealthAttribute());
			SetHealth(FMath::Clamp(NewHealth, ClampMinimumValue, GetMaxHealth()));

			// 대상 코어 컴포넌트를 통해 피해 및 체력 변화 이벤트를 전달합니다.
			if (TargetCoreComponent)
			{
				TargetCoreComponent->HandleDamage(LocalDamageDone, SourceTags, SourceActor);
				TargetCoreComponent->HandleHealthChange(-LocalDamageDone, SourceTags);
			}
		}
	}
}

void UGSCAttributeSet::HandleStaminaDamageAttribute(const FGSCAttributeSetExecutionData& ExecutionData)
{
	UGSCCoreComponent* TargetCoreComponent = ExecutionData.TargetCoreComponent;
	const FGameplayTagContainer SourceTags = ExecutionData.SourceTags;

	// StaminaDamage 값을 로컬 변수에 저장하고, 해당 속성을 0으로 초기화합니다.
	const float LocalStaminaDamageDone = GetStaminaDamage();
	SetStaminaDamage(0.f);

	// 스태미나 피해가 발생했다면, 현재 스태미나에서 차감 후 클램프 처리합니다.
	if (LocalStaminaDamageDone > 0.0f)
	{
		const float NewStamina = GetStamina() - LocalStaminaDamageDone;
		SetStamina(FMath::Clamp(NewStamina, 0.0f, GetMaxStamina()));

		// 대상 코어 컴포넌트를 통해 스태미나 변화 이벤트를 전달합니다.
		if (TargetCoreComponent)
		{
			TargetCoreComponent->HandleStaminaChange(-LocalStaminaDamageDone, SourceTags);
		}
	}
}

void UGSCAttributeSet::HandleHealthAttribute(const FGSCAttributeSetExecutionData& ExecutionData)
{
	UGSCCoreComponent* TargetCoreComponent = ExecutionData.TargetCoreComponent;
	const float ClampMinimumValue = GetClampMinimumValueFor(GetHealthAttribute());

	// 현재 Health 값을 최소값과 최대값 사이로 클램프 처리합니다.
	SetHealth(FMath::Clamp(GetHealth(), ClampMinimumValue, GetMaxHealth()));

	// 대상 코어 컴포넌트를 통해 체력 변화 이벤트를 전달합니다.
	if (TargetCoreComponent)
	{
		const float DeltaValue = ExecutionData.DeltaValue;
		const FGameplayTagContainer SourceTags = ExecutionData.SourceTags;
		TargetCoreComponent->HandleHealthChange(DeltaValue, SourceTags);
	}
}

void UGSCAttributeSet::HandleStaminaAttribute(const FGSCAttributeSetExecutionData& ExecutionData)
{
	UGSCCoreComponent* TargetCoreComponent = ExecutionData.TargetCoreComponent;
	const float ClampMinimumValue = GetClampMinimumValueFor(GetStaminaAttribute());

	SetStamina(FMath::Clamp(GetStamina(), ClampMinimumValue, GetMaxStamina()));

	if (TargetCoreComponent)
	{
		const float DeltaValue = ExecutionData.DeltaValue;
		const FGameplayTagContainer SourceTags = ExecutionData.SourceTags;
		TargetCoreComponent->HandleStaminaChange(DeltaValue, SourceTags);
	}
}

void UGSCAttributeSet::HandleManaAttribute(const FGSCAttributeSetExecutionData& ExecutionData)
{
	UGSCCoreComponent* TargetCoreComponent = ExecutionData.TargetCoreComponent;
	const float ClampMinimumValue = GetClampMinimumValueFor(GetManaAttribute());

	SetMana(FMath::Clamp(GetMana(), ClampMinimumValue, GetMaxMana()));

	if (TargetCoreComponent)
	{
		const float DeltaValue = ExecutionData.DeltaValue;
		const FGameplayTagContainer SourceTags = ExecutionData.SourceTags;
		TargetCoreComponent->HandleManaChange(DeltaValue, SourceTags);
	}
}
