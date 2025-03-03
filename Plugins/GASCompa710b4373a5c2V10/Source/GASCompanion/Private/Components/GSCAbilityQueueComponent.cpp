#include "Components/GSCAbilityQueueComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GSCDelegates.h"
#include "GSCLog.h"
#include "Abilities/GSCGameplayAbility.h"
#include "GameFramework/Pawn.h"


UGSCAbilityQueueComponent::UGSCAbilityQueueComponent()
{
	// PrimaryComponentTick.bCanEverTick를 true로 설정하면, 이 컴포넌트가 매 프레임마다 Tick() 호출을 받게 됩니다.
	// 만약 Tick이 필요하지 않다면 false로 변경하여 성능을 개선할 수 있습니다.
	PrimaryComponentTick.bCanEverTick = true;

	// 네트워크 복제를 활성화합니다.
	SetIsReplicatedByDefault(true);
}

void UGSCAbilityQueueComponent::BeginPlay()
{
	// 부모 클래스 BeginPlay() 호출: 필수 초기화 작업 수행.
	Super::BeginPlay();

	// 소유자(Owner) 관련 정보를 초기화합니다.
	SetupOwner();
}

void UGSCAbilityQueueComponent::SetupOwner()
{
	// 소유 액터가 유효한지 확인합니다.
	if (!GetOwner())
		return; // 소유자가 없으면 초기화 작업을 중단합니다.

	// 소유자가 Pawn이 아니면 이후 로직이 의미 없으므로 중단합니다.
	OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
		return;
	
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPawn);
}

void UGSCAbilityQueueComponent::OpenAbilityQueue()
{
	// 능력 큐 기능이 비활성화되어 있다면 함수 실행 중단.
	if (!bAbilityQueueEnabled)
	{
		return;
	}

	// 큐를 여는 플래그를 true로 설정하여, 이후 능력 큐 등록 로직이 동작하도록 합니다.
	bAbilityQueueOpened = true;
}

void UGSCAbilityQueueComponent::CloseAbilityQueue()
{
	// 능력 큐 기능이 비활성화되어 있다면 함수 실행 중단.
	if (!bAbilityQueueEnabled)
		return;
	
	// 소유 Pawn이 유효하다면, 로그를 통해 큐 종료 상태를 출력합니다.
	if (OwnerPawn)
	{
		// 로그 메시지를 한글로 변경: 현재 Pawn의 이름과 함께 큐를 닫는 상황을 기록합니다.
		GSC_LOG(Verbose, TEXT("UGSCAbilityQueueComponent:CloseAbilityQueue() - 능력 큐를 종료합니다. Pawn 이름: %s"), *OwnerPawn->GetName())
	}

	// 능력 큐 열림 상태 플래그를 false로 설정합니다.
	bAbilityQueueOpened = false;
}

void UGSCAbilityQueueComponent::UpdateAllowedAbilitiesForAbilityQueue(TArray<TSubclassOf<UGameplayAbility>> AllowedAbilities)
{
	// 능력 큐 기능이 활성화되어 있지 않으면 업데이트를 수행하지 않습니다.
	if (!bAbilityQueueEnabled)
		return;
	
	// 입력된 허용 능력 목록을 저장합니다.
	QueuedAllowedAbilities = AllowedAbilities;

	// 디버그 목적으로 사용되는 위젯에 현재 허용된 능력 목록을 갱신합니다.
	UpdateDebugWidgetAllowedAbilities();
}

void UGSCAbilityQueueComponent::SetAllowAllAbilitiesForAbilityQueue(const bool bAllowAllAbilities)
{
	// 능력 큐 기능이 비활성화된 경우 함수 실행 중단.
	if (!bAbilityQueueEnabled)
		return;
	
	// 모든 능력 허용 여부를 설정합니다.
	bAllowAllAbilitiesForAbilityQueue = bAllowAllAbilities;

	// 디버그 위젯 업데이트 호출: 현재 상태를 시각적으로 확인할 수 있도록 합니다.
	UpdateDebugWidgetAllowedAbilities();
}

bool UGSCAbilityQueueComponent::IsAbilityQueueOpened() const
{
	return bAbilityQueueOpened;
}

bool UGSCAbilityQueueComponent::IsAllAbilitiesAllowedForAbilityQueue() const
{
	return bAllowAllAbilitiesForAbilityQueue;
}

const UGameplayAbility* UGSCAbilityQueueComponent::GetCurrentQueuedAbility() const
{
	return QueuedAbility;
}

TArray<TSubclassOf<UGameplayAbility>> UGSCAbilityQueueComponent::GetQueuedAllowedAbilities() const
{
	return QueuedAllowedAbilities;
}

void UGSCAbilityQueueComponent::OnAbilityEnded(const UGameplayAbility* InAbility)
{
	// InAbility를 UGSCGameplayAbility 타입으로 캐스팅 시도.
	if (const UGSCGameplayAbility* CompanionAbility = Cast<UGSCGameplayAbility>(InAbility))
	{
		// 능력 종료 로그 출력 (로그 메시지를 한글로 변경)
		GSC_LOG(
			Verbose,
			TEXT("UGSCAbilityQueueComponent::OnAbilityEnded() - 능력 종료: %s, 능력 큐 활성화: %d, 컴포넌트 큐 활성화: %d"),
			*CompanionAbility->GetName(),
			CompanionAbility->bEnableAbilityQueue ? 1 : -1,
			bAbilityQueueEnabled ? 1 : -1
		);

		// 종료된 능력이 큐 기능을 사용 중이라면, 큐를 닫습니다.
		if (CompanionAbility->bEnableAbilityQueue)
		{
			CloseAbilityQueue();
		}
	}
	else
	{
		// UGSCGameplayAbility가 아닌 경우에도 로그를 출력합니다.
		GSC_LOG(
			Verbose,
			TEXT("UGSCAbilityQueueComponent::OnAbilityEnded() - 능력 종료: %s (UGSCGameplayAbility 아님), 컴포넌트 큐 활성화: %d"),
			*InAbility->GetName(),
			bAbilityQueueEnabled ? 1 : -1
		);

		// 주석 처리된 CloseAbilityQueue() 호출: 필요에 따라 주석 해제 가능.
		// CloseAbilityQueue();
	}

	// 능력 큐 기능이 활성화되어 있다면, 큐에 저장된 능력 처리 진행.
	if (bAbilityQueueEnabled)
	{
		if (QueuedAbility)
		{
			// 현재 큐에 저장된 능력을 지역 변수로 저장 (ResetAbilityQueueState()에서 초기화되므로)
			const UGameplayAbility* AbilityToActivate = QueuedAbility;
			GSC_LOG(Log, TEXT("UGSCAbilityQueueComponent::OnAbilityEnded() - 큐에 등록된 능력 발견: %s [AbilityQueueSystem]"), *AbilityToActivate->GetName());

			// 만약 모든 능력을 허용하거나, 큐에 등록된 능력 클래스가 허용 목록에 있다면
			if (bAllowAllAbilitiesForAbilityQueue || QueuedAllowedAbilities.Contains(AbilityToActivate->GetClass()))
			{
				// 큐 상태 초기화 후 재시도 활성화
				ResetAbilityQueueState();

				GSC_LOG(Log, TEXT("UGSCAbilityQueueComponent::OnAbilityEnded() - %s 은(는) 허용된 능력 목록에 포함됨. 재활성화 시도 [AbilityQueueSystem]"), *AbilityToActivate->GetName());
				if (OwnerAbilitySystemComponent)
				{
					// 능력 시스템 컴포넌트를 통해 해당 능력 클래스로 능력 활성화를 시도합니다.
					OwnerAbilitySystemComponent->TryActivateAbilityByClass(AbilityToActivate->GetClass());
				}
			}
			else
			{
				// 허용되지 않는 능력인 경우 큐 상태만 초기화하고 추가 동작 없이 종료합니다.
				ResetAbilityQueueState();
				GSC_LOG(Verbose, TEXT("UGSCAbilityQueueComponent::OnAbilityEnded() - %s 은(는) 허용되지 않음, 아무 동작도 하지 않음 [AbilityQueueSystem]"), *AbilityToActivate->GetName());
			}
		}
		else
		{
			// 큐에 등록된 능력이 없다면, 내부 상태를 초기화합니다.
			ResetAbilityQueueState();
		}
	}
}

void UGSCAbilityQueueComponent::OnAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& ReasonTags)
{
	// 실패한 능력과 사유에 대한 로그 출력 (로그 메시지 한글로 변경)
	GSC_LOG(Verbose, TEXT("UGSCAbilityQueueComponent::OnAbilityFailed() - 능력 실패: %s, 사유: %s"), *Ability->GetName(), *ReasonTags.ToStringSimple());

	// 능력 큐 기능이 활성화되어 있으며 큐가 열려 있는 경우에만 처리
	if (bAbilityQueueEnabled && bAbilityQueueOpened)
	{
		GSC_LOG(Verbose, TEXT("UGSCAbilityQueueComponent::OnAbilityFailed() - 능력 큐에 등록할 능력 설정: %s"), *Ability->GetName());

		// 허용된 능력인지, 또는 모든 능력이 허용되는 상태인지 검사하여 큐에 등록합니다.
		if (bAllowAllAbilitiesForAbilityQueue || QueuedAllowedAbilities.Contains(Ability->GetClass()))
		{
			// const_cast를 통해 const UGameplayAbility*를 비상수 포인터로 변환하여 QueuedAbility에 할당합니다.
			QueuedAbility = TObjectPtr<UGameplayAbility>(const_cast<UGameplayAbility*>(Ability));
		}
	}
}

void UGSCAbilityQueueComponent::ResetAbilityQueueState()
{
	GSC_LOG(Verbose, TEXT("UGSCAbilityQueueComponent::ResetAbilityQueueState() - 능력 큐 상태 초기화"));
	QueuedAbility = nullptr;
	bAllowAllAbilitiesForAbilityQueue = false;
	QueuedAllowedAbilities.Empty();

	// 디버그 위젯에 현재 상태를 업데이트하여 화면에 반영합니다.
	UpdateDebugWidgetAllowedAbilities();
}

void UGSCAbilityQueueComponent::UpdateDebugWidgetAllowedAbilities()
{
	FGSCDelegates::OnUpdateAllowedAbilities.Broadcast(QueuedAllowedAbilities);
}
