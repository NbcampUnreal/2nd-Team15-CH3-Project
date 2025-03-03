// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GSCTargetType.generated.h"

/**
 * @brief 능력의 타겟팅(대상 결정) 로직을 수행하기 위한 클래스
*
 * 이 클래스는 능력을 사용할 때 어떤 대상을 선택할지 결정하는 로직을 구현하기 위해 사용됩니다.
 * 주로 블루프린트에서 확장되어 구체적인 타겟팅 로직을 작성할 수 있도록 설계되었습니다.
*
 * 주요 특징:
 * - 블루프린트에서 쉽게 상속받아 사용하도록 설계되어 있으며, 타겟팅 로직을 직접 구현할 수 있습니다.
 * - 이 클래스는 GameplayAbilityTargetActor를 상속받지 않으며, 실제 게임 세계에 인스턴스화되지 않습니다.
 *   즉, 이 클래스 자체는 월드에 생성되지 않고, 타겟팅 정보를 계산하기 위한 순수 로직 클래스로 활용됩니다.
 * - 복잡한 타겟팅 로직이 필요한 경우, 이 클래스를 기반으로 게임에 특화된 타겟팅 블루프린트를 생성한 후,  필요에 따라 월드에 인스턴스화하거나 풀링 방식으로 관리할 수 있습니다.
*
 * 사용 예:
 * - 단순한 범위 내의 모든 적 또는 특정 조건을 만족하는 적을 대상으로 하는 타겟팅 로직을 구현할 때
 * - 스킬 사용 시 특정 영역 내의 적들을 자동으로 탐지하여 적용할 대상을 결정할 때
*/
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class GASCOMPANION_API UGSCTargetType : public UObject
{
	GENERATED_BODY()

public:
	// 생성자 및 기본 오버라이드
	UGSCTargetType()
	{
	}

	/**
	 * @brief 능력 효과를 적용할 타겟을 결정하는 함수
	 *
	 * 이 함수는 블루프린트에서 구현되며, 타겟팅 액터(TargetingActor)와 이벤트 데이터를 기반으로
	 * 효과를 적용할 대상(AActor)과 히트 결과(FHitResult)를 출력 배열에 채워 반환합니다.
	 *
	 * @param TargetingActor 능력 타겟팅을 수행하는 액터 (예: 스킬을 사용하는 캐릭터)
	 * @param EventData 해당 능력 사용 시 발생한 이벤트 데이터 (추가적인 타겟팅 조건 등을 포함할 수 있음)
	 * @param OutHitResults 능력 효과 적용을 위해 히트 판정 결과를 담을 배열 (예: 레이 캐스팅 결과)
	 * @param OutActors 능력 효과를 적용할 대상을 담을 액터 배열
	 */
	UFUNCTION(BlueprintNativeEvent)
	void GetTargets(AActor* TargetingActor, FGameplayEventData EventData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};
