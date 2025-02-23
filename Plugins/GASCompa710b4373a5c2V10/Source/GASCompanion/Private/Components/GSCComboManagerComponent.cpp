// Copyright 2021 Mickael Daniel. All Rights Reserved.

#include "Components/GSCComboManagerComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Abilities/GSCGameplayAbility_MeleeBase.h"
#include "Components/GSCCoreComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GSCLog.h"

UGSCComboManagerComponent::UGSCComboManagerComponent()
{
	// 컴포넌트가 매 프레임마다 Tick 함수를 호출할지 여부를 지정합니다.
	PrimaryComponentTick.bCanEverTick = true;
	
	// 기본적으로 네트워크 복제를 사용하도록 설정합니다.
	SetIsReplicatedByDefault(true);

	// 기본 근접 공격으로 GSCGameplayAbility_MeleeBase를 지정합니다.
	MeleeBaseAbility = UGSCGameplayAbility_MeleeBase::StaticClass();
}


void UGSCComboManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 부모 클래스의 복제 설정 호출
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UGSCComboManagerComponent, ComboIndex, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSCComboManagerComponent, bComboWindowOpened, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSCComboManagerComponent, bShouldTriggerCombo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSCComboManagerComponent, bRequestTriggerCombo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UGSCComboManagerComponent, bNextComboAbilityActivated, COND_None, REPNOTIFY_Always);
}

void UGSCComboManagerComponent::IncrementCombo()
{
	// 콤보 윈도우가 열려있는지 확인
	if (bComboWindowOpened)
	{
		// 콤보 인덱스를 증가
		ComboIndex = ComboIndex + 1;
	}
}

void UGSCComboManagerComponent::ResetCombo()
{
	// 로그를 통해 콤보 리셋 수행을 알립니다. (한글화)
	GSC_LOG(Verbose, TEXT("UGSCComboManagerComponent::ResetCombo() - 콤보를 초기화합니다."));

	// 콤보 인덱스를 0으로 설정
	SetComboIndex(0);
}

void UGSCComboManagerComponent::ActivateComboAbility(const TSubclassOf<UGSCGameplayAbility> AbilityClass, const bool bAllowRemoteActivation)
{
	// 오너 액터가 서버 권한을 가지고 있는지 확인
	if (IsOwnerActorAuthoritative())
	{
		// 서버 권한이 있으면 내부 함수 직접 호출
		ActivateComboAbilityInternal(AbilityClass, bAllowRemoteActivation);
	}
	else
	{
		// 권한이 없으면, 서버로 RPC 요청
		ServerActivateComboAbility(AbilityClass, bAllowRemoteActivation);
	}
}

void UGSCComboManagerComponent::SetComboIndex(const int32 InComboIndex)
{
	if (IsOwnerActorAuthoritative())
	{
		// 서버 권한이 있는 경우 직접 설정
		ComboIndex = InComboIndex;
	}
	else
	{
		// 권한이 없더라도 임시로 로컬에 설정하고, 서버에 반영 요청
		ComboIndex = InComboIndex;
		ServerSetComboIndex(InComboIndex);
	}
}

bool UGSCComboManagerComponent::IsOwnerActorAuthoritative() const
{
	return !bCachedIsNetSimulated;
}

void UGSCComboManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// 오너(Owner) 관련 초기화
	SetupOwner();

	// 클라이언트에서 맵 배치된 액터일 경우를 대비해, 네트워크 역할을 재확인하여 캐싱
	CacheIsNetSimulated();
}

void UGSCComboManagerComponent::OnRegister()
{
	Super::OnRegister();

	// 서버 또는 클라이언트 중 어느 역할인지 한 번 더 캐싱
	CacheIsNetSimulated();
}

void UGSCComboManagerComponent::SetupOwner()
{
	if (!GetOwner())
	{
		return;
	}

	// 소유자를 ACharacter로 캐스팅
	OwningCharacter = Cast<ACharacter>(GetOwner());
	if (!OwningCharacter)
	{
		return;
	}

	// GSCBlueprintFunctionLibrary를 통해 CompanionCoreComponent를 획득
	OwnerCoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(OwningCharacter);
}

UGameplayAbility* UGSCComboManagerComponent::GetCurrentActiveComboAbility() const
{
	if (!OwnerCoreComponent)
	{
		GSC_LOG(Error, TEXT("UGSCComboManagerComponent::GetCurrentActiveComboAbility() - CompanionCoreComponent가 유효하지 않습니다."));
		return nullptr;
	}

	// MeleeBaseAbility로 활성화된 능력을 조회
	TArray<UGameplayAbility*> Abilities = OwnerCoreComponent->GetActiveAbilitiesByClass(MeleeBaseAbility);

	// 첫 번째 능력(0번 인덱스) 반환, 없으면 nullptr
	return Abilities.IsValidIndex(0) ? Abilities[0] : nullptr;
}

void UGSCComboManagerComponent::ActivateComboAbilityInternal(const TSubclassOf<UGSCGameplayAbility> AbilityClass, const bool bAllowRemoteActivation)
{
	// 일단 bShouldTriggerCombo를 false로 초기화
	bShouldTriggerCombo = false;

	// OwningCharacter가 유효한지 체크
	if (!OwningCharacter)
	{
		GSC_LOG(Error, TEXT("UGSCComboManagerComponent::ActivateComboAbilityInternal() - OwningCharacter가 null입니다."));
		return;
	}

	// OwnerCoreComponent가 유효한지 체크
	if (!OwnerCoreComponent)
	{
		GSC_LOG(
			Error,
			TEXT("UGSCComboManagerComponent::ActivateComboAbilityInternal() - OwnerCoreComponent가 null입니다. ")
			TEXT("GSCComboManagerComponent가 정상 동작하려면 GSCCoreComponent가 필요합니다.\n\n")
			TEXT("'%s'에 Blueprint Components로 추가되어 있는지 확인해 주세요."),
			*GetNameSafe(GetOwner())
		);
		return;
	}

	// AbilityClass가 유효한지 체크
	if (!AbilityClass)
	{
		GSC_LOG(Error, TEXT("UGSCComboManagerComponent::ActivateComboAbilityInternal() - AbilityClass가 null입니다."));
		return;
	}

	// 이미 AbilityClass에 해당하는 능력을 사용 중인지 확인
	if (OwnerCoreComponent->IsUsingAbilityByClass(AbilityClass))
	{
		GSC_LOG(
			Verbose,
			TEXT("UGSCComboManagerComponent::ActivateComboAbilityInternal() - %s는 이미 %s 능력을 사용 중입니다. ")
			TEXT("bComboWindowOpened가 true라면 콤보를 잇습니다."),
			*GetName(),
			*AbilityClass->GetName()
		);

		// 콤보 윈도우가 열려있다면, 다음 콤보 트리거 플래그를 활성화
		bShouldTriggerCombo = bComboWindowOpened;
	}
	else
	{
		// 능력이 활성화되지 않았다면 새로 활성화 시도
		GSC_LOG(Verbose, TEXT("UGSCComboManagerComponent::ActivateComboAbilityInternal() - %s에서 %s 능력을 새로 활성화합니다."), *GetName(), *AbilityClass->GetName());

		UGSCGameplayAbility* TempActivateAbility;
		OwnerCoreComponent->ActivateAbilityByClass(AbilityClass, TempActivateAbility, bAllowRemoteActivation);
	}
}

void UGSCComboManagerComponent::CacheIsNetSimulated()
{
	// IsNetSimulating()이 true라면, 이 액터는 클라이언트 측(비권한)입니다.
	bCachedIsNetSimulated = IsNetSimulating();
}

void UGSCComboManagerComponent::ServerActivateComboAbility_Implementation(const TSubclassOf<UGSCGameplayAbility> AbilityClass, const bool bAllowRemoteActivation)
{
	// 서버에서 Multicast를 호출하여 모든 클라이언트에 알림
	MulticastActivateComboAbility(AbilityClass, bAllowRemoteActivation);
}

void UGSCComboManagerComponent::MulticastActivateComboAbility_Implementation(const TSubclassOf<UGSCGameplayAbility> AbilityClass, const bool bAllowRemoteActivation)
{
	// 로컬로 컨트롤되지 않는(Authority가 아닌) 캐릭터에서만 실행
	if (OwningCharacter && !OwningCharacter->IsLocallyControlled())
	{
		ActivateComboAbilityInternal(AbilityClass, bAllowRemoteActivation);
	}
}


void UGSCComboManagerComponent::ServerSetComboIndex_Implementation(const int32 InComboIndex)
{
	// 서버에서 Multicast를 호출하여 모든 클라이언트에 알림
	MulticastSetComboIndex(InComboIndex);
}

void UGSCComboManagerComponent::MulticastSetComboIndex_Implementation(const int32 InComboIndex)
{
	// 로컬로 컨트롤되지 않는 캐릭터인 경우에만 콤보 인덱스를 반영
	if (OwningCharacter && !OwningCharacter->IsLocallyControlled())
	{
		ComboIndex = InComboIndex;
	}
}
