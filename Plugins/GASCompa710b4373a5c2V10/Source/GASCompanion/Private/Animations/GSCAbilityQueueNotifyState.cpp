// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Animations/GSCAbilityQueueNotifyState.h"

#include "GSCDelegates.h"
#include "GSCLog.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/SkeletalMeshComponent.h"


void UGSCAbilityQueueNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	// 로그 출력: 노티파이 시작
	GSC_LOG(Log, TEXT("UGSCAbilityQueueNotifyState:NotifyBegin()"))

	// MeshComp로부터 소유 액터를 가져옵니다.
	const AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// 소유 액터에서 AbilityQueueComponent를 검색합니다.
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Owner);
	if (!AbilityQueueComponent)
	{
		return;
	}

	// AbilityQueueComponent가 활성화되어 있는지 확인합니다.
	if (!AbilityQueueComponent->bAbilityQueueEnabled)
	{
		return;
	}

	// 로그 출력: 큐잉 가능한 능력 개수를 로그로 출력 (디버깅 목적)
	GSC_LOG(Log, TEXT("UGSCAbilityQueueNotifyState:NotifyBegin() Open Ability Queue for %d allowed abilities"), AllowedAbilities.Num())
	
	// 큐 창을 열고, 모든 능력 큐잉 허용 여부 및 허용 목록을 설정합니다.
	AbilityQueueComponent->OpenAbilityQueue();
	AbilityQueueComponent->SetAllowAllAbilitiesForAbilityQueue(bAllowAllAbilities);
	AbilityQueueComponent->UpdateAllowedAbilitiesForAbilityQueue(AllowedAbilities);

	// 디버그 위젯(예: UI)에 알리기 위해 델리게이트를 브로드캐스트합니다.
	FGSCDelegates::OnAddAbilityQueueFromMontageRow.Broadcast(Animation);
}

void UGSCAbilityQueueNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	// 로그 출력: 노티파이 종료
	GSC_LOG(Log, TEXT("UGSCAbilityQueueNotifyState:NotifyEnd()"))

	// MeshComp로부터 소유 액터를 검색합니다.
	const AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// 소유 액터에서 AbilityQueueComponent를 검색합니다.
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Owner);
	if (!AbilityQueueComponent)
	{
		return;
	}

	// AbilityQueueComponent가 활성화되어 있지 않으면 처리를 중단합니다.
	if (!AbilityQueueComponent->bAbilityQueueEnabled)
	{
		return;
	}

	// AbilityQueueComponent의 큐 창을 닫습니다.
	AbilityQueueComponent->CloseAbilityQueue();
}

FString UGSCAbilityQueueNotifyState::GetNotifyName_Implementation() const
{
	return "[C++]AbilityQueueWindow";
}
