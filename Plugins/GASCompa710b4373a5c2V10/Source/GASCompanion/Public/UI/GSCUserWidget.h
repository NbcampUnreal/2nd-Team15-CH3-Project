// Copyright 2021 Mickael Daniel. All Rights Reserved.

/*
    [전체 코드 개요]
    이 헤더 파일은 언리얼 엔진의 UserWidget을 상속받은 UGSCUserWidget 클래스와
    FGSCGameplayEffectUIData 구조체를 정의합니다.
    GAS(Gameplay Ability System)와 연동되는 위젯으로, 능력 시스템 관련 이벤트(어트리뷰트 변경, 
    이펙트 추가/제거, 쿨다운 등)에 반응하여 UI를 업데이트할 수 있도록 도와주는 역할을 합니다.
    즉, 플레이어 HUD나 기타 UI 요소에서 능력 시스템 데이터를 시각화하고 제어하기 위한
    기능들을 제공하는 기반 클래스입니다.
*/

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffectTypes.h"
#include "GSCUserWidget.generated.h"

class UGSCCoreComponent;

/*
    [구조체: FGSCGameplayEffectUIData]
    - 이 구조체는 GameplayEffect(게임플레이 이펙트)의 UI 표시에 필요한 데이터를 묶어둔 것입니다.
    - 효과가 적용된 시점(StartTime), 총 지속 시간(TotalDuration), 종료 시점(ExpectedEndTime),
      그리고 스택(StackCount), 스택 제한(StackLimitCount) 등의 정보를 포함합니다.
    - UI에서 게임플레이 이펙트의 남은 시간, 스택 수 등을 시각화할 때 유용합니다.
    - 예: 스킬 사용 시 일정 시간 동안 효과가 유지되는 버프 아이콘의 남은 시간을 표시하거나,
          스택 기반 버프의 스택 갯수를 표시하는 등의 UI 로직에 활용할 수 있습니다.
*/
USTRUCT(BlueprintType)
struct FGSCGameplayEffectUIData
{
	GENERATED_BODY()

	/*
	    [StartTime]
	    - 이펙트가 처음 적용된 시점(초 단위)
	    - 서버 또는 클라이언트에서 이펙트가 시작된 WorldTime 등을 기준으로 설정됩니다.
	    - UI에서는 효과가 언제 시작되었는지 계산하여 남은 시간을 표시하거나,
	      경과 시간을 시각화할 때 활용할 수 있습니다.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	float StartTime;

	/*
	    [TotalDuration]
	    - 이펙트가 유지되는 총 시간(초 단위)
	    - 0이면 무한 지속이거나, 특정한 종료 조건이 따로 있는 경우일 수 있습니다.
	    - 남은 시간을 계산하거나, 쿨다운 표시 등에 사용됩니다.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	float TotalDuration;

	/*
	    [ExpectedEndTime]
	    - 이펙트가 종료될 것으로 예상되는 시점(초 단위)
	    - StartTime + TotalDuration 계산값이 들어가는 경우가 많으며,
	      중간에 리프레시(Refresh)나 갱신이 이루어지면 다시 계산될 수 있습니다.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	float ExpectedEndTime;

	/*
	    [StackCount]
	    - 현재 이펙트의 스택 개수
	    - 동일한 이펙트가 여러 번 중첩되었을 때 몇 번이나 적용되었는지를 나타냅니다.
	    - 예: 버프 효과가 누적될 때, 스택 UI를 갱신하여 플레이어에게 시각적으로 알립니다.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	int32 StackCount;

	/*
	    [StackLimitCount]
	    - 이펙트가 최대 몇 스택까지 누적될 수 있는지 나타내는 값
	    - 게임플레이 디자이너가 설정한 값이며, 이 스택 개수를 초과하지 않습니다.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="GAS Companion|GameplayEffect")
	int32 StackLimitCount;

	// 구조체의 전체 파라미터를 명시적으로 설정하는 생성자입니다.
	FGSCGameplayEffectUIData(const float InStartTime, const float InTotalDuration, const float InExpectedEndTime, const int32 InStackCount, const int32 InStackLimitCount)
		: StartTime(InStartTime),
		  TotalDuration(InTotalDuration),
		  ExpectedEndTime(InExpectedEndTime),
		  StackCount(InStackCount),
		  StackLimitCount(InStackLimitCount)
	{
	}

	// 디폴트 생성자로, 초기값이 모두 0으로 설정됩니다.
	FGSCGameplayEffectUIData(): StartTime(0), TotalDuration(0), ExpectedEndTime(0), StackCount(0), StackLimitCount(0)
	{
	}
};

/**
 * [클래스: UGSCUserWidget]
 *
 * 언리얼 엔진의 UUserWidget를 상속받은 UI 클래스입니다.
 * Ability System과 연동되도록 설계되었으며, GAS(Gameplay Ability System)에서 발생하는
 * 이벤트(어트리뷰트 변경, Gameplay Tag 변화, Gameplay Effect 추가/제거, 쿨다운 시작/종료 등)를
 * 수신하여 UI를 갱신하거나 관련 로직을 처리합니다.
 *
 * 주로 다음과 같은 기능을 제공합니다:
 * - AbilitySystemComponent(ASC)에 대한 참조 캐싱 및 초기화
 * - 어트리뷰트 변경, 게임플레이 이펙트 추가/제거 시 BlueprintImplementableEvent를 통해 UI 업데이트
 * - 쿨다운 발생 시점과 종료 시점을 받아 HUD에 표시
 *
 * 플레이어 HUD 등에서 상속받아 사용하면, 게임플레이 능력 시스템과 UI를 쉽게 연동할 수 있습니다.
 * 예: 체력바 갱신, 버프/디버프 아이콘 표시, 스킬 쿨다운 타이머 표시 등에 활용합니다.
 */
UCLASS()
class GASCOMPANION_API UGSCUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/*
	    [SetOwnerActor]
	    - 위젯의 소유 액터(OwnerActor)를 설정하며, 동시에 CoreComponent와 AbilitySystemComponent도 찾습니다.
	    - 주로 다른 클래스에서 이 위젯을 생성할 때, '어떤 액터를 기준으로 표시될 것인지'를 설정하기 위해 사용합니다.
	*/
	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual void SetOwnerActor(AActor* Actor);

	/*
	    [GetOwningActor]
	    - 이 위젯이 현재 소유하는 액터를 반환합니다.
	    - OwnerActor 프로퍼티 대신 사용하도록 권장됩니다.
	    - Blueprint에서 쉽게 불러 쓸 수 있도록 BlueprintCallable로 설정되어 있습니다.
	*/
	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual AActor* GetOwningActor() const { return OwnerActor; }

	/*
	    [GetOwningCoreComponent]
	    - 이 위젯이 현재 소유하는 UGSCCoreComponent를 반환합니다.
	    - OwnerCoreComponent 프로퍼티 대신 사용하도록 권장됩니다.
	    - Blueprint에서 쉽게 불러 쓸 수 있도록 BlueprintCallable로 설정되어 있습니다.
	*/
	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual UGSCCoreComponent* GetOwningCoreComponent() const { return OwnerCoreComponent; }

	/*
	    [GetOwningAbilitySystemComponent]
	    - 이 위젯이 현재 참조하고 있는 AbilitySystemComponent를 반환합니다.
	    - OwnerActor에서 가져오며, InitializeWithAbilitySystem 함수를 통해 캐싱됩니다.
	    - UI에서 능력 시스템 데이터를 조회하거나 바인딩할 때 사용합니다.
	*/
	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual UAbilitySystemComponent* GetOwningAbilitySystemComponent() const { return AbilitySystemComponent; }

	/*
	    [InitializeWithAbilitySystem]
	    - 외부에서 AbilitySystemComponent(ASC)를 전달받아 위젯을 초기화합니다.
	    - SetOwnerActor가 호출되지 않았다면 자동으로 ASC의 OwnerActor를 Owner로 설정합니다.
	    - 기존에 등록된 ASC가 있다면 ResetAbilitySystem()을 통해 정리한 후 새롭게 설정합니다.
	    - ASC 관련 델리게이트를 바인딩하고, OnAbilitySystemInitialized 이벤트를 통해
	      Blueprint 측에서도 초기화 후 처리를 할 수 있도록 합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual void InitializeWithAbilitySystem(UPARAM(ref) const UAbilitySystemComponent* AbilitySystemComponent);

	/*
	    [ResetAbilitySystem]
	    - 기존 ASC에 바인딩된 모든 델리게이트를 해제(ShutdownAbilitySystemComponentListeners)하고
	      AbilitySystemComponent 포인터를 null로 설정합니다.
	    - 새로운 ASC를 세팅하기 전에 깨끗이 정리할 때 사용합니다.
	*/
	void ResetAbilitySystem();

	/*
	    [RegisterAbilitySystemDelegates]
	    - ASC(AbilitySystemComponent)에서 발생하는 여러 이벤트에 대해 델리게이트를 등록합니다.
	    - 예: 어트리뷰트 변경, GameplayEffect 추가, Tag 변경, Ability 커밋 등.
	    - ASC가 존재하지 않을 수 있으므로, nullptr 체크 후에 작업을 수행합니다.
	*/
	virtual void RegisterAbilitySystemDelegates();

	/*
	    [ShutdownAbilitySystemComponentListeners]
	    - 이미 ASC에 등록한 델리게이트들을 모두 제거하여 정리합니다.
	    - 위젯이 제거되거나, 새로운 ASC로 교체될 때 사용합니다.
	*/
	virtual void ShutdownAbilitySystemComponentListeners() const;

	/*
	    [OnAbilitySystemInitialized]
	    - ASC가 정상적으로 초기화된 시점에 Blueprint에서 처리할 로직을 정의하기 위한 이벤트입니다.
	    - BlueprintImplementableEvent로 선언되어 있어, C++에서는 구현 없이 블루프린트에서 구현합니다.
	    - 어트리뷰트 초기값 가져오기, UI 초기 상태 셋업 등에 사용합니다.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnAbilitySystemInitialized();

	/*
	    [OnAttributeChange]
	    - ASC에서 어트리뷰트가 변경될 때 호출되는 이벤트입니다.
	    - Blueprint에서 구현하며, UI갱신이나 사운드/이펙트 재생 등의 처리를 넣을 수 있습니다.
	    - Attribute: 어떤 어트리뷰트가 변경되었는지 (예: Health)
	    - NewValue: 변경된 최종 값
	    - OldValue: 변경 이전 값
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnAttributeChange(FGameplayAttribute Attribute, float NewValue, float OldValue);

	/*
	    [OnGameplayEffectStackChange]
	    - GameplayEffect 스택이 변경될 때(추가, 감소 등) 호출되는 이벤트입니다.
	    - Blueprint에서 구현하며, UI에서 버프 아이콘 스택 표시 등을 업데이트할 때 사용됩니다.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayEffectStackChange(
		FGameplayTagContainer AssetTags,
		FGameplayTagContainer GrantedTags,
		FActiveGameplayEffectHandle ActiveHandle,
		int32 NewStackCount,
		int32 OldStackCount
	);

	/*
	    [OnGameplayEffectTimeChange]
	    - GameplayEffect의 지속시간이 변경될 때(갱신, 연장 등) 호출되는 이벤트입니다.
	    - Blueprint에서 구현하여, UI의 남은 시간 표시 등을 실시간으로 업데이트할 수 있습니다.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayEffectTimeChange(
		FGameplayTagContainer AssetTags,
		FGameplayTagContainer GrantedTags,
		FActiveGameplayEffectHandle ActiveHandle,
		float NewStartTime,
		float NewDuration
	);

	/*
	    [OnGameplayEffectAdded]
	    - 새로운 GameplayEffect가 적용되었을 때(즉, 추가되었을 때) 호출되는 이벤트입니다.
	    - Blueprint에서 구현하여, 버프 아이콘 추가 등 UI 측 처리를 담당합니다.
	    - EffectData 구조체를 통해 시작 시간, 지속 시간, 스택 등을 알 수 있습니다.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayEffectAdded(
		FGameplayTagContainer AssetTags,
		FGameplayTagContainer GrantedTags,
		FActiveGameplayEffectHandle ActiveHandle,
		FGSCGameplayEffectUIData EffectData
	);

	/*
	    [OnGameplayEffectRemoved]
	    - 기존에 적용되어 있던 GameplayEffect가 제거되었을 때 호출되는 이벤트입니다.
	    - Blueprint에서 구현하여, 버프 아이콘 제거 등 UI 처리를 담당합니다.
	    - EffectData 구조체를 통해 삭제 시점의 스택이나 시간 정보를 확인할 수 있습니다.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayEffectRemoved(
		FGameplayTagContainer AssetTags,
		FGameplayTagContainer GrantedTags,
		FActiveGameplayEffectHandle ActiveHandle,
		FGSCGameplayEffectUIData EffectData
	);

	/*
	    [OnGameplayTagChange]
	    - GameplayTag가 새롭게 추가되거나 전부 제거될 때(Count가 1->0 또는 0->1로 바뀔 때) 호출되는 이벤트입니다.
	    - Tag가 증가/감소만 한 경우(예: 2->3)는 호출되지 않습니다.
	    - Blueprint에서 구현하여 태그 상태가 변경되었을 때 UI 변화를 처리합니다.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnGameplayTagChange(FGameplayTag GameplayTag, int32 NewTagCount);

	/*
	    [OnCooldownStart]
	    - 어떤 Ability가 쿨다운에 들어갈 때(Ability 사용 시) 호출되는 이벤트입니다.
	    - Blueprint에서 구현하여, 해당 Ability에 대한 쿨다운 UI를 표시하거나 갱신할 수 있습니다.
	    - TimeRemaining: 쿨다운이 끝날 때까지 남은 시간
	    - Duration: 전체 쿨다운 시간
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnCooldownStart(UGameplayAbility* Ability, const FGameplayTagContainer CooldownTags, float TimeRemaining, float Duration);

	/*
	    [OnCooldownEnd]
	    - Ability 쿨다운이 종료될 때 호출되는 이벤트입니다.
	    - Blueprint에서 구현하여, 해당 Ability 쿨다운 UI를 제거하거나 "준비 완료" 표시 등의 로직을 처리합니다.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "GAS Companion|UI")
	void OnCooldownEnd(UGameplayAbility* Ability, FGameplayTag CooldownTag, float Duration);

	/*
	    [GetPercentForAttributes]
	    - 두 개의 어트리뷰트(일반 값과 최대 값)에 대한 퍼센트(0.0 ~ 1.0 범위)를 구합니다.
	    - 예: Health / MaxHealth, Stamina / MaxStamina 등.
	    - 분모가 0인 경우(최대값이 0)가 되지 않도록 주의하며, 0을 반환하도록 처리합니다.
	*/
	UFUNCTION(BlueprintPure, Category="GAS Companion|UI")
	float GetPercentForAttributes(FGameplayAttribute Attribute, FGameplayAttribute MaxAttribute) const;

	/*
	    [GetAttributeValue]
	    - 주어진 어트리뷰트의 현재 값을 ASC로부터 가져옵니다.
	    - ASC가 없거나, 해당 어트리뷰트가 유효하지 않은 경우 0을 반환합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="GAS Companion|UI")
	virtual float GetAttributeValue(const FGameplayAttribute Attribute) const;

	/*
	    [OnAttributeChanged]
	    - 어트리뷰트 변경 시 ASC에서 호출되며, 내부적으로 OnAttributeChange 이벤트를 BP에 브로드캐스트합니다.
	    - HandleAttributeChange 함수를 통해 C++ 하위 클래스에서도 추가 처리를 할 수 있습니다.
	*/
	virtual void OnAttributeChanged(const FOnAttributeChangeData& Data);

	/*
	    [OnActiveGameplayEffectAdded]
	    - ASC에 GameplayEffect가 새로 추가될 때 호출되며,
	      StackChange, TimeChange 델리게이트도 바인딩합니다.
	    - HandleGameplayEffectAdded를 통해 Blueprint 이벤트(OnGameplayEffectAdded)를 호출합니다.
	*/
	virtual void OnActiveGameplayEffectAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveHandle);

	/*
	    [OnActiveGameplayEffectStackChanged]
	    - ASC에 등록된 GameplayEffect가 스택 변화를 일으킬 때 호출됩니다.
	    - HandleGameplayEffectStackChange를 통해 Blueprint 이벤트(OnGameplayEffectStackChange)를 호출합니다.
	*/
	virtual void OnActiveGameplayEffectStackChanged(FActiveGameplayEffectHandle ActiveHandle, int32 NewStackCount, int32 PreviousStackCount);

	/*
	    [OnActiveGameplayEffectTimeChanged]
	    - ASC에 등록된 GameplayEffect의 시간 정보가 변경될 때(예: 지속 시간이 갱신될 때) 호출됩니다.
	    - HandleGameplayEffectTimeChange를 통해 Blueprint 이벤트(OnGameplayEffectTimeChange)를 호출합니다.
	*/
	virtual void OnActiveGameplayEffectTimeChanged(FActiveGameplayEffectHandle ActiveHandle, float NewStartTime, float NewDuration);

	/*
	    [OnAnyGameplayEffectRemoved]
	    - ASC에서 GameplayEffect가 제거될 때 호출됩니다.
	    - HandleGameplayEffectRemoved를 통해 Blueprint 이벤트(OnGameplayEffectRemoved)를 호출합니다.
	*/
	virtual void OnAnyGameplayEffectRemoved(const FActiveGameplayEffect& EffectRemoved);

	/*
	    [OnAnyGameplayTagChanged]
	    - ASC에서 GameplayTag가 추가 또는 제거될 때 호출됩니다.(NewCount가 1->0, 0->1)
	    - HandleGameplayTagChange를 통해 Blueprint 이벤트(OnGameplayTagChange)를 호출합니다.
	*/
	virtual void OnAnyGameplayTagChanged(FGameplayTag GameplayTag, int32 NewCount);

	/*
	    [OnAbilityCommitted]
	    - ASC에서 Ability가 실제로 사용되어(Commit) 자원 소모나 쿨다운 등이 적용되었을 때 호출됩니다.
	    - HandleCooldownStart를 호출하여 Blueprint 이벤트(OnCooldownStart)로 전달합니다.
	*/
	virtual void OnAbilityCommitted(UGameplayAbility* ActivatedAbility);

	/*
	    [OnCooldownGameplayTagChanged]
	    - Ability 쿨다운 태그가 변경될 때(추가/삭제) 호출됩니다.
	    - 쿨다운이 종료되어 태그가 제거될 경우, HandleCooldownEnd를 호출하여 Blueprint 이벤트(OnCooldownEnd)로 전달합니다.
	*/
	virtual void OnCooldownGameplayTagChanged(const FGameplayTag GameplayTag, int32 NewCount, FGameplayAbilitySpecHandle AbilitySpecHandle, float Duration);

	/*
	    [HandleAttributeChange]
	    - C++ 하위 클래스에서 어트리뷰트 변경 시 추가 처리를 할 수 있도록 오버라이드 가능하도록 만들어진 함수입니다.
	    - 기본 구현은 비어 있으며, 필요에 따라 오버라이드하여 사용합니다.
	*/
	virtual void HandleAttributeChange(FGameplayAttribute Attribute, float NewValue, float OldValue)
	{
	}

	/*
	    [HandleGameplayEffectStackChange]
	    - C++ 하위 클래스에서 스택 변경 시 추가 처리를 할 수 있도록 오버라이드 가능하도록 만든 함수입니다.
	    - OnGameplayEffectStackChange 이벤트를 호출합니다.
	*/
	virtual void HandleGameplayEffectStackChange(
		FGameplayTagContainer AssetTags,
		FGameplayTagContainer GrantedTags,
		FActiveGameplayEffectHandle ActiveHandle,
		int32 NewStackCount,
		int32 OldStackCount
	);

	/*
	    [HandleGameplayEffectTimeChange]
	    - C++ 하위 클래스에서 시간 변경 시 추가 처리를 할 수 있도록 오버라이드 가능하도록 만든 함수입니다.
	    - OnGameplayEffectTimeChange 이벤트를 호출합니다.
	*/
	virtual void HandleGameplayEffectTimeChange(
		FGameplayTagContainer AssetTags,
		FGameplayTagContainer GrantedTags,
		FActiveGameplayEffectHandle ActiveHandle,
		float NewStartTime,
		float NewDuration
	);

	/*
	    [HandleGameplayEffectAdded]
	    - C++ 하위 클래스에서 이펙트 추가 시 추가 처리를 할 수 있도록 오버라이드 가능하도록 만든 함수입니다.
	    - OnGameplayEffectAdded 이벤트를 호출합니다.
	*/
	virtual void HandleGameplayEffectAdded(
		FGameplayTagContainer AssetTags,
		FGameplayTagContainer GrantedTags,
		FActiveGameplayEffectHandle ActiveHandle
	);

	/*
	    [HandleGameplayEffectRemoved]
	    - C++ 하위 클래스에서 이펙트 제거 시 추가 처리를 할 수 있도록 오버라이드 가능하도록 만든 함수입니다.
	    - OnGameplayEffectRemoved 이벤트를 호출합니다.
	*/
	virtual void HandleGameplayEffectRemoved(
		FGameplayTagContainer AssetTags,
		FGameplayTagContainer GrantedTags,
		FActiveGameplayEffectHandle ActiveHandle
	);

	/*
	    [HandleGameplayTagChange]
	    - C++ 하위 클래스에서 태그 변화 시 추가 처리를 할 수 있도록 오버라이드 가능하도록 만든 함수입니다.
	    - OnGameplayTagChange 이벤트를 호출합니다.
	*/
	virtual void HandleGameplayTagChange(FGameplayTag GameplayTag, int32 NewTagCount);

	/*
	    [HandleCooldownStart]
	    - Ability가 커밋되어 쿨다운이 시작될 때, 쿨다운 UI 갱신 처리를 담당합니다.
	    - OnCooldownStart 이벤트를 호출하여 Blueprint 측에서 추가 로직을 구현할 수 있도록 합니다.
	*/
	virtual void HandleCooldownStart(UGameplayAbility* Ability, const FGameplayTagContainer CooldownTags, float TimeRemaining, float Duration);

	/*
	    [HandleCooldownEnd]
	    - Ability 쿨다운 태그가 제거되어 쿨다운이 종료될 때, 쿨다운 관련 UI 해제 처리를 담당합니다.
	    - OnCooldownEnd 이벤트를 호출하여 Blueprint 측에서 추가 로직을 구현할 수 있도록 합니다.
	*/
	virtual void HandleCooldownEnd(UGameplayAbility* Ability, FGameplayTag CooldownTag, float Duration);

	/*
	    [GetGameplayEffectUIData]
	    - FActiveGameplayEffectHandle을 통해 UI 표시용 FGSCGameplayEffectUIData를 생성해 반환합니다.
	    - UAbilitySystemBlueprintLibrary를 이용하여 이펙트 시작/종료 시간, 스택 정보 등을 조회합니다.
	*/
	FGSCGameplayEffectUIData GetGameplayEffectUIData(FActiveGameplayEffectHandle ActiveHandle);

public:
	/*
		[OwnerActor]
		- 현재 위젯이 소유하고 있는 액터에 대한 참조입니다.
		- DeprecatedFunction으로 표시되어 있으며, 대신 GetOwningActor() 함수를 사용하도록 권장됩니다.
		- 이 위젯이 표시되는 대상(플레이어 캐릭터 등)을 가리키며, 이를 통해 AbilitySystemComponent를 찾아냅니다.
	*/
	UPROPERTY(BlueprintReadOnly, Category="GAS Companion|UI", meta=(DeprecatedFunction, DeprecationMessage="Use GetOwningActor() instead."))
	TObjectPtr<AActor> OwnerActor;

	/*
		[OwnerCoreComponent]
		- Companion Core Component에 대한 참조이며, Ability System과 상호작용하는 핵심 로직이 들어 있습니다.
		- DeprecatedFunction으로 표시되어 있으며, 대신 GetOwningCoreComponent() 함수를 사용하도록 권장됩니다.
		- OwnerActor에 부착된 UGSCCoreComponent를 저장합니다.
	*/
	UPROPERTY(BlueprintReadOnly, Category="GAS Companion|UI", meta=(DeprecatedFunction, DeprecationMessage="Use GetOwningCoreComponent() instead."))
	TObjectPtr<UGSCCoreComponent> OwnerCoreComponent;

protected:
	/*
	    [AbilitySystemComponent]
	    - 현재 위젯에서 참조하는 ASC로, 주로 OwnerActor 또는 SetOwnerActor 함수를 통해 설정됩니다.
	    - 어트리뷰트나 GameplayEffect 데이터 조회, 이벤트 바인딩에 사용됩니다.
	*/
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

private:
	/*
	    [GameplayEffectAddedHandles]
	    - OnActiveGameplayEffectAddedDelegateToSelf로 등록된 모든 GameplayEffect의 핸들을 저장합니다.
	    - 차후에 ShutdownAbilitySystemComponentListeners에서 일괄적으로 델리게이트를 제거하기 위해 활용됩니다.
	*/
	TArray<FActiveGameplayEffectHandle> GameplayEffectAddedHandles;

	/*
	    [GameplayTagBoundToDelegates]
	    - OnCooldownGameplayTagChanged 같은 태그 변화 관련 델리게이트 등록 시 사용한 태그들을 저장합니다.
	    - ShutdownAbilitySystemComponentListeners에서 델리게이트를 일괄 제거하기 위해 활용됩니다.
	*/
	TArray<FGameplayTag> GameplayTagBoundToDelegates;
};
