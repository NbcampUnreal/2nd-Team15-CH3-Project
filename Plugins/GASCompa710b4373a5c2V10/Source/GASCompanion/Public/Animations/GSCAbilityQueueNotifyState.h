#pragma once

#include "CoreMinimal.h"
#include "Abilities/GSCGameplayAbility.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GSCAbilityQueueNotifyState.generated.h"

/*
===========================================================================================
  파일 개요:
    UGSCAbilityQueueNotifyState 클래스는 애니메이션 노티파이 스테이트(AnimNotifyState)를 확장하여,
    몽타주(애니메이션 시퀀스) 재생 중 능력 큐(Ability Queue) 창을 열고 닫는 기능을 제공합니다.
    이 노티파이를 사용하면, Gameplay Ability 내부에서 몽타주가 재생되는 동안
    활성화에 실패한 능력들을 큐에 등록하여 현재 능력이 종료된 후 순차적으로 활성화할 수 있습니다.
===========================================================================================
*/

/*------------------------------------------------------------------------------
  UGSCAbilityQueueNotifyState 클래스 선언
------------------------------------------------------------------------------*/

/**
 * @brief 능력 큐 창 노티파이 스테이트 클래스.
 *
 * 이 클래스는 몽타주 애니메이션의 특정 구간(콤보 윈도우) 동안 능력 큐 창을 열어,
 * 해당 구간에 능력 활성화에 실패한 능력을 큐잉할 수 있도록 합니다.
 *
 * 기본 기능:
 *  - 몽타주 시작 시(NotifyBegin) 능력 큐 창을 열고, 허용할 능력 목록을 설정합니다.
 *  - 몽타주 종료 시(NotifyEnd) 능력 큐 창을 닫습니다.
 *  - GetNotifyName_Implementation을 통해 에디터와 디버깅 시 표시될 이름을 제공합니다.
 *
 * meta 속성의 DisplayName을 "AbilityQueueWindow"로 설정하여, 에디터에서 인식하기 쉽게 합니다.
 */
UCLASS(meta = (DisplayName = "AbilityQueueWindow"))
class GASCOMPANION_API UGSCAbilityQueueNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//-------------------------------------
	// UAnimNotifyState 인터페이스 함수 (오버라이드)
	//-------------------------------------
	/**
	 * @brief 노티파이 시작 시 호출되는 함수.
	 *
	 * 몽타주 애니메이션이 시작될 때, 이 함수가 호출되어 능력 큐 컴포넌트를 검색한 후,
	 * 큐잉 창을 열고 허용된 능력 목록을 업데이트합니다.
	 *
	 * @param MeshComp 노티파이가 발생한 스켈레탈 메쉬 컴포넌트.
	 * @param Animation 현재 재생 중인 애니메이션 시퀀스.
	 * @param TotalDuration 몽타주의 전체 재생 시간.
	 */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	/**
	 * @brief 노티파이 종료 시 호출되는 함수.
	 *
	 * 몽타주 애니메이션의 해당 구간이 종료되면, 이 함수가 호출되어 능력 큐 컴포넌트에서
	 * 큐잉 창을 닫는 처리를 수행합니다.
	 *
	 * @param MeshComp 노티파이가 발생한 스켈레탈 메쉬 컴포넌트.
	 * @param Animation 현재 재생 중인 애니메이션 시퀀스.
	 */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	/**
	 * @brief 노티파이 이름 반환 함수.
	 *
	 * 이 함수는 에디터 및 디버깅 시, 노티파이의 이름을 확인하기 위해 사용됩니다.
	 *
	 * @return "AbilityQueueWindow" 문자열을 반환합니다.
	 */
	virtual FString GetNotifyName_Implementation() const override;

public:
	//-------------------------------------
	// 공개 멤버 변수 (능력 큐 설정)
	//-------------------------------------
	/**
	 * @brief 모든 능력 큐잉 허용 여부.
	 *
	 * true일 경우, AllowedAbilities 배열에 관계없이 모든 능력이 큐잉됩니다.
	 * false이면 AllowedAbilities 배열에 명시된 능력만 큐잉됩니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	bool bAllowAllAbilities = false;

	/**
	 * @brief 큐잉 가능한 능력 목록.
	 * bAllowAllAbilities가 false일 경우에만 적용되며, 이 배열에 포함된 UGameplayAbility의 서브클래스만 큐잉할 수 있습니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	TArray<TSubclassOf<UGameplayAbility>> AllowedAbilities;
};
