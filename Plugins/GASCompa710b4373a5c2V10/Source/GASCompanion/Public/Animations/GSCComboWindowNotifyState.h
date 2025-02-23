
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/GSCComboManagerComponent.h"
#include "GSCComboWindowNotifyState.generated.h"

/*
===========================================================================================
  파일 개요:
    UGSCComboWindowNotifyState 클래스는 애니메이션 노티파이 스테이트(AnimNotifyState)를 확장하여,
    몽타주(애니메이션 시퀀스) 재생 중에 능력 큐(Ability Queue) 창을 열고 닫는 기능을 제공합니다.
    이 노티파이를 통해 플레이어는 콤보 윈도우 동안 다음 콤보를 이어가기 위한 입력을 등록할 수 있습니다.
    단, 마지막 몽타주인 경우에는 bEndCombo 플래그를 true로 설정하여 콤보 종료 처리를 수행해야 합니다.
===========================================================================================
*/


class UGSCComboManagerComponent;


/**
 * @brief 능력 큐 창 노티파이 스테이트 클래스.
 *
 * 이 클래스는 몽타주 애니메이션의 특정 구간(예: 콤보 윈도우) 동안 능력 큐 창을 열어,
 * 플레이어가 콤보 입력을 등록할 수 있도록 합니다.
 * - bEndCombo 값이 true이면, 해당 몽타주가 콤보 체인의 마지막임을 나타내며, 콤보를 리셋하는 처리를 수행합니다.
 */
UCLASS()
class GASCOMPANION_API UGSCComboWindowNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	//-------------------------------------
	// 공개 멤버 함수 (UAnimNotifyState 인터페이스 오버라이드)
	//-------------------------------------
public:
	/**
	 * @brief 노티파이 시작 시 호출되는 함수.
	 *
	 * 몽타주 애니메이션의 콤보 창 구간이 시작되면 호출되며,
	 * 소유 액터의 AbilityQueueComponent를 검색하여 콤보 창 상태를 열어줍니다.
	 *
	 * @param MeshComp 노티파이가 발생한 스켈레탈 메쉬 컴포넌트.
	 * @param Animation 현재 재생 중인 애니메이션 시퀀스.
	 * @param TotalDuration 노티파이 구간의 전체 지속 시간.
	 */
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;

	/**
	 * @brief 노티파이 종료 시 호출되는 함수.
	 *
	 * 몽타주 애니메이션의 콤보 창 구간이 종료되면 호출되어,
	 * 소유 액터의 AbilityQueueComponent를 통해 콤보 상태를 초기화하고 창을 닫습니다.
	 *
	 * @param MeshComp 노티파이가 발생한 스켈레탈 메쉬 컴포넌트.
	 * @param Animation 현재 재생 중인 애니메이션 시퀀스.
	 */
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	/**
	 * @brief 노티파이 Tick 시 호출되는 함수.
	 *
	 * 매 프레임 호출되며, 능력 큐 창이 열려있는 동안 입력 등록 및 능력 활성화를 위한 추가 처리를 수행합니다.
	 *
	 * @param MeshComp 노티파이가 발생한 스켈레탈 메쉬 컴포넌트.
	 * @param Animation 현재 재생 중인 애니메이션 시퀀스.
	 * @param FrameDeltaTime 현재 프레임의 델타 시간.
	 */
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
	
	virtual FString GetEditorComment() override;
	
	virtual FString GetNotifyName_Implementation() const override;

private:
	/**
	 * @brief 애니메이션 편집기 미리보기 액터 식별 문자열.
	 *
	 * 이 문자열은 소유 액터가 Persona의 애니메이션 편집기 미리보기 액터인지 판별하는 데 사용됩니다.
	 * 미리보기 액터인 경우, 실제 게임 로직 실행을 방지하여 불필요한 로그 경고를 막습니다.
	 */
	FString AnimationEditorPreviewActorString = "AnimationEditorPreviewActor";

	/**
	 * @brief 스켈레탈 메쉬 컴포넌트의 소유 액터를 검색합니다.
	 *
	 * MeshComponent로부터 소유 액터를 얻은 후, 액터 이름이 AnimationEditorPreviewActorString으로 시작하면
	 * nullptr를 반환하여 미리보기 액터에서는 게임 로직이 실행되지 않도록 합니다.
	 *
	 * @param MeshComponent 소유 액터를 검색할 스켈레탈 메쉬 컴포넌트.
	 * @return 유효한 소유 액터 포인터, 조건에 맞지 않으면 nullptr.
	 */
	AActor* GetOwnerActor(USkeletalMeshComponent* MeshComponent) const;

public:
	/**
	 * @brief 콤보 종료 여부.
	 *
	 * 이 값이 true이면, 현재 몽타주가 콤보 체인의 마지막 몽타주임을 의미하며, 능력 큐 종료 시 콤보 리셋을 수행합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimNotify")
	bool bEndCombo = false;	
};

