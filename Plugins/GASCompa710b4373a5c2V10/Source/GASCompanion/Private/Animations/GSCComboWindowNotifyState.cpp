// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Animations/GSCComboWindowNotifyState.h"

#include "GSCLog.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Abilities/GSCGameplayAbility.h"
#include "Components/GSCComboManagerComponent.h"
#include "Components/GSCCoreComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UGSCComboWindowNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	// MeshComp에서 소유 액터를 가져옵니다.
	const AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		// 소유 액터가 없으면 처리를 중단합니다.
		return;
	}

	// 서버에서만 실행하도록 확인: 클라이언트는 권한(Authority)을 가지지 않으므로 처리하지 않습니다.
	if (!Owner->HasAuthority())
	{
		return;
	}

	if (UGSCComboManagerComponent* ComboManagerComponent = UGSCBlueprintFunctionLibrary::GetComboManagerComponent(Owner))
	{
		ComboManagerComponent->bComboWindowOpened = true;
	}
}

void UGSCComboWindowNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	const AActor* Owner = GetOwnerActor(MeshComp);
	if (!Owner)
	{
		return;
	}

	// run only on server
	if (!Owner->HasAuthority())
	{
		return;
	}

	if (UGSCComboManagerComponent* ComboManagerComponent = UGSCBlueprintFunctionLibrary::GetComboManagerComponent(Owner))
	{
		GSC_LOG(Verbose, TEXT("NotifyEnd: bNextComboAbilityActivated %s (%s)"), ComboManagerComponent->bNextComboAbilityActivated ? TEXT("true") : TEXT("false"), *Owner->GetName())
		GSC_LOG(Verbose, TEXT("NotifyEnd: bEndCombo %s (%s)"), bEndCombo ? TEXT("true") : TEXT("false"), *Owner->GetName())
		if (!ComboManagerComponent->bNextComboAbilityActivated || bEndCombo)
		{
			GSC_LOG(Verbose, TEXT("NotifyEnd: ResetCombo  (%s)"), *Owner->GetName())
			ComboManagerComponent->ResetCombo();
		}

		ComboManagerComponent->bComboWindowOpened = false;
		ComboManagerComponent->bRequestTriggerCombo = false;
		ComboManagerComponent->bShouldTriggerCombo = false;
		ComboManagerComponent->bNextComboAbilityActivated = false;
	}
}

void UGSCComboWindowNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime)
{
	const AActor* Owner = GetOwnerActor(MeshComp);
	if (!Owner)
	{
		return;
	}

	// run only on server
	if (!Owner->HasAuthority())
	{
		return;
	}

	UGSCComboManagerComponent* ComboManagerComponent = UGSCBlueprintFunctionLibrary::GetComboManagerComponent(Owner);
	if (!ComboManagerComponent)
	{
		return;
	}

	if (ComboManagerComponent->bComboWindowOpened && ComboManagerComponent->bShouldTriggerCombo && ComboManagerComponent->bRequestTriggerCombo && !bEndCombo)
	{
		UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Owner);
		// prevent reactivate of ability in this tick window (especially on networked environment with some lags)
		if (CoreComponent && !ComboManagerComponent->bNextComboAbilityActivated)
		{
			if (const UGameplayAbility* ComboAbility = ComboManagerComponent->GetCurrentActiveComboAbility())
			{
				UGSCGameplayAbility* ActivatedAbility;
				if (CoreComponent->ActivateAbilityByClass(ComboAbility->GetClass(), ActivatedAbility))
				{
					ComboManagerComponent->bNextComboAbilityActivated = true;
				}
				else
				{
					GSC_LOG(Verbose, TEXT("ComboWindowNotifyState:NotifyTick Ability %s didn't activate"), *ComboAbility->GetClass()->GetName())
				}
			}
		}
	}
}

FString UGSCComboWindowNotifyState::GetEditorComment()
{
	return TEXT("probably not yet implemented");
}

FString UGSCComboWindowNotifyState::GetNotifyName_Implementation() const
{
	return bEndCombo ? "[C++]GSC Combo Window (ending)     " : "[C++]GSC Combo Window    ";
}

AActor* UGSCComboWindowNotifyState::GetOwnerActor(USkeletalMeshComponent* MeshComponent) const
{
	AActor* OwnerActor = MeshComponent->GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	const FString ActorName = OwnerActor->GetName();
	if (ActorName.StartsWith(AnimationEditorPreviewActorString))
	{
		return nullptr;
	}

	return OwnerActor;
}
