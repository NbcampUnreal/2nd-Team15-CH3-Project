// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameplayTagContainer.h"
#include "GSCTypes.generated.h"

/*
===========================================================================================
  파일 개요:
    이 파일은 GAS Companion 시스템에서 사용되는 핵심 타입들을 정의합니다.
    여기에는 능력 활성화 입력 트리거 이벤트를 나타내는 열거형, 
    그리고 GameplayEffect를 적용하기 위한 컨테이너 및 해당 컨테이너의 런타임 사양(Processed Spec)
    를 나타내는 구조체들이 포함됩니다.
===========================================================================================
*/

/*------------------------------------------------------------------------------
  전방 선언
------------------------------------------------------------------------------*/
// 아래 클래스들은 이 파일에서 사용되며, 실제 정의는 다른 헤더 파일에 있습니다.
class UGSCTargetType;    // 타겟팅 방식(Targeting) 관련 클래스. 타겟을 선택하는 방법을 정의.
class UGameplayEffect;    // GameplayEffect를 나타내며, 능력 실행 시 적용되는 효과를 정의합니다.

/*------------------------------------------------------------------------------
  열거형: 능력 활성화 입력 트리거 이벤트
------------------------------------------------------------------------------*/

/**
 * @brief 능력 활성화 입력 트리거 이벤트 열거형.
 *
 * 이 열거형은 능력 활성화 시 어떤 입력 이벤트를 기반으로 활성화할지를 정의합니다.
 * - Started: 버튼을 누르자마자 한 번만 발생하는 이벤트 (일반적인 경우 권장)
 * - Triggered: 입력을 지속하는 동안 매 프레임 발생하는 이벤트 (연속 입력에 주의하여 사용)
 */
UENUM(BlueprintType)
enum class EGSCAbilityTriggerEvent : uint8
{
	/** 
	 * 버튼을 누르는 즉시 한 번만 발생하는 이벤트.
	 * 대부분의 액션에 적합하며, 즉각적인 능력 활성화에 사용됩니다.
	 */
	Started UMETA(DisplayName="Activate on Action Started (recommended)"),

	/**
	 * 입력을 유지하는 동안 매 프레임 발생하는 이벤트.
	 * 주의: 이 값은 입력이 매 틱마다 발생할 경우 능력 활성화에 문제를 일으킬 수 있으므로,
	 * 단 한 번만 발생하도록 보장된 액션에 사용해야 합니다.
	 */
	Triggered UMETA(DisplayName="Activate on Action Triggered (use with caution)"),
};


/*------------------------------------------------------------------------------
  구조체: FGSCGameplayEffectContainer
------------------------------------------------------------------------------*/

/**
 * @brief 효과 컨테이너 구조체.
*
 * 이 구조체는 Blueprint나 에셋에서 정적으로 정의되며, 
 * 런타임 시 효과 사양으로 변환되어 능력 실행 시 적용될 GameplayEffect들을 그룹화합니다.
 *
 * 주요 구성 요소:
 *  - TargetType: 타겟팅 방식을 지정하는 클래스.
 *  - TargetGameplayEffectClasses: 타겟에게 적용할 GameplayEffect 클래스들의 배열.
 *  - bUseSetByCallerMagnitude: SetByCaller Magnitude 사용 여부를 결정.
 *  - SetByCallerDataTag: SetByCaller Magnitude를 지정할 때 참조할 태그.
 *  - SetByCallerMagnitude: 기본 Magnitude 값 (보통 1.0f).
*/
USTRUCT(BlueprintType)
struct FGSCGameplayEffectContainer
{
	GENERATED_BODY()

public:
	FGSCGameplayEffectContainer() {}

	/** 타겟팅 방식을 설정하는 UGSCTargetType의 서브클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TSubclassOf<UGSCTargetType> TargetType;

	/** 타겟에게 적용할 GameplayEffect 클래스들의 리스트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<TSubclassOf<UGameplayEffect>> TargetGameplayEffectClasses;

	/** SetByCaller Magnitude 사용 여부; true이면 효과의 Magnitude를 외부에서 지정할 수 있음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	bool bUseSetByCallerMagnitude = false;

	/** SetByCaller Magnitude를 지정할 때 사용할 데이터 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	FGameplayTag SetByCallerDataTag;

	/** SetByCaller Magnitude의 기본값 (기본적으로 1.0f) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	float SetByCallerMagnitude = 1.0f;
};


/*------------------------------------------------------------------------------
  구조체: FGSCGameplayEffectContainerSpec
------------------------------------------------------------------------------*/

/**
 * @brief 효과 컨테이너 사양 구조체.
 *
 * 이 구조체는 FGSCGameplayEffectContainer의 런타임 버전으로,
 * 실제 효과 적용 전에 계산된 타겟 데이터와 효과 사양 핸들을 포함합니다.
 * 이 사양은 능력 실행 시 최종적으로 GameplayEffect를 대상에게 적용하는 데 사용됩니다.
 */
USTRUCT(BlueprintType)
struct GASCOMPANION_API FGSCGameplayEffectContainerSpec
{
	GENERATED_BODY()

public:
	FGSCGameplayEffectContainerSpec() {}

	/** 계산된 타겟 데이터: 효과를 적용할 대상들의 정보를 담고 있음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	FGameplayAbilityTargetDataHandle TargetData;

	/** 타겟에게 적용할 효과 사양 핸들의 리스트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GameplayEffectContainer)
	TArray<FGameplayEffectSpecHandle> TargetGameplayEffectSpecs;

	/**
	 * @brief 효과 사양이 유효한지 확인합니다.
	 *
	 * @return TargetGameplayEffectSpecs 배열에 하나 이상의 유효한 효과 사양이 있으면 true.
	 */
	bool HasValidEffects() const;

	/**
	 * @brief 타겟 데이터가 유효한지 확인합니다.
	 *
	 * @return TargetData에 하나 이상의 유효한 타겟 정보가 있으면 true.
	 */
	bool HasValidTargets() const;

	/**
	 * @brief 새로운 타겟 데이터를 추가합니다.
	 *
	 * Hit 결과와 대상 액터 배열을 기반으로 타겟 데이터를 생성하여 TargetData에 추가합니다.
	 * 이 함수는 두 종류의 타겟 데이터를 처리합니다:
	 *  - FGameplayAbilityTargetData_SingleTargetHit: Hit 결과 기반 단일 타겟.
	 *  - FGameplayAbilityTargetData_ActorArray: 액터 배열 기반 다중 타겟.
	 *
	 * @param HitResults 충돌(Hit) 결과 배열.
	 * @param TargetActors 대상 액터 배열.
	 */
	void AddTargets(const TArray<FHitResult>& HitResults, const TArray<AActor*>& TargetActors);
};

