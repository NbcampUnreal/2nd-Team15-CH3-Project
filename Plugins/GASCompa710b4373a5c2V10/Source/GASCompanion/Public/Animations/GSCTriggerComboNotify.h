
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GSCTriggerComboNotify.generated.h"

/*
===========================================================================================
  파일 개요:
    UGSCTriggerComboNotify 클래스는 애니메이션 노티파이(AnimNotify)를 확장하여,
    플레이어가 콤보 윈도우 동안 입력을 등록한 경우, 시스템에 다음 콤보 활성화를 트리거하도록  알리는 기능을 제공합니다.
    이 노티파이는 반드시 콤보 윈도우 영역 내에 배치되어야 올바르게 동작합니다.
===========================================================================================
*/

/*------------------------------------------------------------------------------
  UGSCTriggerComboNotify 클래스 선언
------------------------------------------------------------------------------*/

/**
 * @brief 콤보 트리거 노티파이 클래스.
 *
 * 이 클래스는 애니메이션 노티파이(AnimNotify)를 상속받아, 콤보 윈도우 내에서 플레이어 입력을
 * 감지하고 콤보 활성화를 요청하는 기능을 제공합니다.
 * - 서버에서만 동작하도록 설계되어 있으며,
 * - Persona 편집기 미리보기 액터에서는 동작하지 않아 불필요한 로그 경고를 방지합니다.
 */
UCLASS()
class GASCOMPANION_API UGSCTriggerComboNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	/**
	 * @brief 애니메이션 노티파이 이벤트 처리 함수.
	 *
	 * 애니메이션 재생 중 노티파이가 발생하면 호출되어, 콤보 활성화를 위한 조건을 확인하고,
	 * 서버 권한이 있는 경우 콤보 매니저 컴포넌트에 트리거 요청을 전달합니다.
	 *
	 * @param MeshComp 애니메이션을 재생 중인 스켈레탈 메쉬 컴포넌트.
	 * @param Animation 현재 재생 중인 애니메이션 시퀀스.
	 */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	/**
	 * @brief 노티파이의 이름을 반환하는 함수 (블루프린트용).
	 *
	 * 이 함수는 애니메이션 에디터나 디버깅 시, 노티파이의 이름을 확인하기 위해 사용됩니다.
	 *
	 * @return 노티파이의 이름을 나타내는 FString.
	 */
	virtual FString GetNotifyName_Implementation() const override;

private:
	/**
	 * @brief 애니메이션 에디터 미리보기 액터 이름 문자열.
	 *
	 * 이 문자열은 현재 노티파이의 소유 액터가 Persona의 애니메이션 편집기 미리보기 액터인지 확인할 때 사용됩니다.
	 * 미리보기 액터의 경우 콤보 요청을 수행하지 않도록 하여 불필요한 로그 경고를 방지합니다.
	 */
	FString AnimationEditorPreviewActorString = "AnimationEditorPreviewActor";

	/**
	 * @brief 스켈레탈 메쉬 컴포넌트의 소유 액터를 반환합니다.
	 *
	 * 주어진 MeshComponent의 소유 액터를 검색한 후, 액터 이름이 미리보기 액터 이름으로 시작하면 nullptr를 반환합니다.
	 * 그렇지 않으면 소유 액터 포인터를 반환합니다.
	 *
	 * @param MeshComponent 소유 액터를 검색할 스켈레탈 메쉬 컴포넌트.
	 * @return 유효한 소유 액터 포인터, 조건 미충족 시 nullptr.
	 */
	AActor* GetOwnerActor(USkeletalMeshComponent* MeshComponent) const;
};
