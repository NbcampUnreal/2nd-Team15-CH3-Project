// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GSCTargetType.h"
#include "GSCTargetTypeUseOwner.generated.h"

/**
 * @brief 소유자(Owner)를 타겟으로 사용하는 간단한 타겟팅 클래스
 *
 * 이 클래스는 능력을 사용할 때 기본적으로 능력을 실행하는 액터 자신을 타겟으로 설정하는
 * 타겟팅 로직을 구현합니다. 복잡한 타겟팅 로직이 필요하지 않은 경우에 사용되며, 주로 능력을 발동한 액터가 스스로를 대상으로 효과를 적용할 때 사용됩니다.
 *
 * 주요 특징:
 * - 블루프린트에서 확장할 필요 없이, 기본적인 타겟팅 로직으로 "자기 자신"을 반환합니다.
 * - 이 클래스는 실제 게임 월드에 인스턴스화되지 않으며, 단순 로직 실행을 위한 UObject 기반의 클래스입니다.
 *
 * 사용 예:
 * - 플레이어가 스스로 버프를 적용하거나, 자신에게 치유 효과를 부여할 때
 * - 특정 상황에서 능력 발동 시 대상이 별도로 필요 없는 경우에 활용할 수 있습니다.
 */
UCLASS(NotBlueprintable)
class GASCOMPANION_API UGSCTargetTypeUseOwner : public UGSCTargetType
{
	GENERATED_BODY()

public:
	// Constructor and overrides
	UGSCTargetTypeUseOwner()
	{
	}

	//----------------------------------------
	// 타겟팅 함수
	//----------------------------------------
	/**
	 * @brief 능력 효과를 적용할 대상을 결정하는 함수의 구현
	 *
	 * 이 함수는 능력을 발동할 때 호출되며, 전달된 이벤트 데이터와 타겟팅 액터 정보를 기반으로 효과를 적용할 대상을 결정합니다.
	 * UGSCTargetTypeUseOwner는 단순히 능력을 실행하는 액터 자신을 대상으로 반환합니다.
	 *
	 * @param TargetingActor 능력 타겟팅을 수행하는 액터 (보통 능력을 발동한 액터 자신)
	 * @param EventData 능력 발동과 관련된 이벤트 데이터 (추가 정보 및 타겟 조건을 포함할 수 있음)
	 * @param OutHitResults 히트 결과 배열 (레이 캐스팅 등으로 검출된 대상 정보를 담을 수 있으나, 이 클래스에서는 사용하지 않음)
	 * @param OutActors 실제 효과를 적용할 액터 배열; 이 함수에서는 TargetingActor 자체를 반환합니다.
	 */
	virtual void GetTargets_Implementation(AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};
