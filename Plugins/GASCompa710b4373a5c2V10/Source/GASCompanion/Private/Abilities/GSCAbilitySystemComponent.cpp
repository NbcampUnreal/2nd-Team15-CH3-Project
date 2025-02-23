#include "Abilities/GSCAbilitySystemComponent.h"

#include "GSCLog.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Abilities/GSCGameplayAbility_MeleeBase.h"
#include "Animation/AnimInstance.h"
#include "Animations/GSCNativeAnimInstanceInterface.h"
#include "Components/GSCAbilityInputBindingComponent.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Components/GSCComboManagerComponent.h"
#include "Components/GSCCoreComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Runtime/Launch/Resources/Version.h"


void UGSCAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1) Ability 활성/종료/실패 발생 시, 해당 함수를 콜백으로 등록
	AbilityActivatedCallbacks.AddUObject(this, &UGSCAbilitySystemComponent::OnAbilityActivatedCallback);
	AbilityFailedCallbacks.AddUObject(this, &UGSCAbilitySystemComponent::OnAbilityFailedCallback);
	AbilityEndedCallbacks.AddUObject(this, &UGSCAbilitySystemComponent::OnAbilityEndedCallback);

	// 2) BeginPlay 시점에 GrantedEffects를 적용 (InitAbilityActorInfo에서 적용하면 Blueprint 미리보기 시점에 Effects가 틱을 돌 수 있으므로 여기서 처리)
	GrantStartupEffects();
}

void UGSCAbilitySystemComponent::BeginDestroy()
{
	// 기본 BeginDestroy 호출 전, 등록된 리소스/Delegate를 정리

	// 1) Pawn Controller가 변경될 때 등록한 Delegate 제거
	if (AbilityActorInfo && AbilityActorInfo->OwnerActor.IsValid())
	{
		if (UGameInstance* GameInstance = AbilityActorInfo->OwnerActor->GetGameInstance())
		{
			GameInstance->GetOnPawnControllerChanged().RemoveAll(this);
		}
	}

	// 2) OnGiveAbility Delegate들 제거
	OnGiveAbilityDelegate.RemoveAll(this);

	// 3) 부여된 AttributeSet 인스턴스 제거
	for (UAttributeSet* AttribSetInstance : AddedAttributes)
	{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
		RemoveSpawnedAttribute(AttribSetInstance);
#else
		GetSpawnedAttributes_Mutable().Remove(AttribSetInstance);
#endif
	}

	// 4) 부여된 Ability들에 대해 입력 바인딩 해제
	UGSCAbilityInputBindingComponent* InputComponent = AbilityActorInfo && AbilityActorInfo->AvatarActor.IsValid()
		                                                   ? AbilityActorInfo->AvatarActor->FindComponentByClass<UGSCAbilityInputBindingComponent>()
		                                                   : nullptr;

	for (const FGSCMappedAbility& DefaultAbilityHandle : AddedAbilityHandles)
	{
		if (InputComponent)
		{
			InputComponent->ClearInputBinding(DefaultAbilityHandle.Handle);
		}

		// 오직 서버 권한에서만 Ability 제거 처리
		if (IsOwnerActorAuthoritative())
		{
			SetRemoveAbilityOnEnd(DefaultAbilityHandle.Handle);
		}
	}

	// 5) CoreComponent에 등록된 Delegate 해제
	UGSCCoreComponent* CoreComponent = AbilityActorInfo && AbilityActorInfo->AvatarActor.IsValid()
		                                   ? AbilityActorInfo->AvatarActor->FindComponentByClass<UGSCCoreComponent>()
		                                   : nullptr;
	if (CoreComponent)
	{
		CoreComponent->ShutdownAbilitySystemDelegates(this);
	}

	// 6) 내부 캐싱한 정보 초기화
	AddedAbilityHandles.Reset();
	AddedAttributes.Reset();
	AddedEffects.Reset();
	AddedAbilitySets.Reset();

	Super::BeginDestroy();
}

void UGSCAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	GSC_WLOG(Verbose,
	         TEXT("Owner: %s, Avatar: %s, World: %s (IsGameWorld: %s)"),
	         *GetNameSafe(InOwnerActor),
	         *GetNameSafe(InAvatarActor),
	         *GetNameSafe(GetWorld()),
	         GetWorld()->IsGameWorld() ? TEXT("true") : TEXT("false")
	)

	// Editor Preview나 Thumbnail 같은 곳에서 ASC 초기화가 불필요하므로, 실제 GameWorld일 때만 진행
	if (GetWorld() && !GetWorld()->IsGameWorld())
	{
		return;
	}

	if (AbilityActorInfo && InOwnerActor)
	{
		// 필요 시 AnimInstance 값 초기화
		if (AbilityActorInfo->AnimInstance == nullptr)
		{
			AbilityActorInfo->AnimInstance = AbilityActorInfo->GetAnimInstance();
		}

		// Pawn Controller 변경 이벤트 등록 (PossessedBy 시 등)
		if (UGameInstance* GameInstance = InOwnerActor->GetGameInstance())
		{
			if (!GameInstance->GetOnPawnControllerChanged().Contains(this, TEXT("OnPawnControllerChanged")))
			{
				GameInstance->GetOnPawnControllerChanged().AddDynamic(this, &UGSCAbilitySystemComponent::OnPawnControllerChanged);
			}
		}

		// AnimInstance가 GSCNativeAnimInstanceInterface를 구현한다면, ASC와 연동
		UAnimInstance* AnimInstance = AbilityActorInfo->GetAnimInstance();
		if (IGSCNativeAnimInstanceInterface* AnimInstanceInterface = Cast<IGSCNativeAnimInstanceInterface>(AnimInstance))
		{
			GSC_WLOG(Verbose, TEXT("AnimInstance `%s`와 AbilitySystem 연결 초기화"), *GetNameSafe(AnimInstance))
			AnimInstanceInterface->InitializeWithAbilitySystem(this);
		}
	}

	// 1) 기본 Ability/Attribute 부여
	GrantDefaultAbilitiesAndAttributes(InOwnerActor, InAvatarActor);

	// 2) 기본 Ability Set 부여
	GrantDefaultAbilitySets(InOwnerActor, InAvatarActor);

	// 만약 CoreComponent가 있다면, 소유자 초기화 및 델리게이트 등록
	UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(InAvatarActor);
	if (CoreComponent)
	{
		CoreComponent->SetupOwner();
		CoreComponent->RegisterAbilitySystemDelegates(this);
		CoreComponent->SetStartupAbilitiesGranted(true);
	}

	// 3) Blueprint에서 ASC 초기화를 감지하기 위한 델리게이트 호출 (클라이언트/서버 모두)
	OnInitAbilityActorInfo.Broadcast();
	if (CoreComponent)
	{
		CoreComponent->OnInitAbilityActorInfo.Broadcast();
	}
}

/**
 * @brief 입력이 눌렸을 때(버튼 Down 등) 호출되는 함수
 *
 * @param InputID 입력 식별자 (ASC에서 지정)
 *
 * - GenericConfirm/Cancel 입력이면 LocalInputConfirm/Cancel 호출
 * - Ability가 콤보형일 경우 콤보 매니저 활성
 * - 일반 Ability는 TryActivateAbility, 혹은 이미 활성화된 경우 AbilitySpecInputPressed
 */
void UGSCAbilitySystemComponent::AbilityLocalInputPressed(const int32 InputID)
{
	// 1) GenericConfirm/Cancel 핸들링 여부 검사
	if (IsGenericConfirmInputBound(InputID))
	{
		LocalInputConfirm();
		return;
	}
	if (IsGenericCancelInputBound(InputID))
	{
		LocalInputCancel();
		return;
	}

	// 2) 일반적인 Ability 입력 처리
	ABILITYLIST_SCOPE_LOCK(); // ActivatableAbilities 목록 접근 시 스레드 안전
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		if (Spec.InputID == InputID && Spec.Ability)
		{
			Spec.InputPressed = true; // 스펙에 입력 눌림 상태 표시

			// 2-1) Ability가 GSCGameplayAbility_MeleeBase(콤보)인지 확인
			if (Spec.Ability->IsA(UGSCGameplayAbility_MeleeBase::StaticClass()))
			{
				// 콤보 매니저 컴포넌트 참조가 없으면 찾아서 설정
				if (!IsValid(ComboComponent))
				{
					ComboComponent = UGSCBlueprintFunctionLibrary::GetComboManagerComponent(GetAvatarActor());
					if (ComboComponent)
					{
						ComboComponent->SetupOwner();
					}
				}

				// 콤보 매니저가 유효하면 콤보 활성화 시도
				if (IsValid(ComboComponent))
				{
					ComboComponent->ActivateComboAbility(Spec.Ability->GetClass());
				}
				else
				{
					GSC_LOG(Error, TEXT("콤보 매니저 컴포넌트가 없어 콤보 Ability 활성화 불가능합니다. Blueprint에서 콤보 매니저 컴포넌트를 추가해주세요."));
				}
			}
			else
			{
				// 2-2) 일반 Ability
				if (Spec.IsActive())
				{
					// 서버가 아닌 경우, ReplicateInputDirectly가 true면 서버에 입력 전달
					if (Spec.Ability->bReplicateInputDirectly && IsOwnerActorAuthoritative() == false)
					{
						ServerSetInputPressed(Spec.Handle);
					}

					// AbilitySpecInputPressed 호출 (Ability 내부에서 InputPressed 처리)
					AbilitySpecInputPressed(Spec);

					PRAGMA_DISABLE_DEPRECATION_WARNINGS
					// InstancedPerExecution이면 신뢰도 문제가 있으므로 경고 로그
					TArray<UGameplayAbility*> Instances = Spec.GetAbilityInstances();
					const FGameplayAbilityActivationInfo& ActivationInfo = Instances.IsEmpty()
						                                                       ? Spec.ActivationInfo
						                                                       : Instances.Last()->GetCurrentActivationInfoRef();
					PRAGMA_ENABLE_DEPRECATION_WARNINGS

					// InputPressed 이벤트 호출 (ReplicatedEvent는 별도 전송 없음, 필요 시 사용 측에서 처리)
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, ActivationInfo.GetActivationPredictionKey());
				}
				else
				{
					// Ability가 활성화되지 않았다면 TryActivateAbility
					TryActivateAbility(Spec.Handle);
				}
			}
		}
	}
}

/**
 * @brief [DEPRECATED] ASC에 Ability를 부여하는 헬퍼 함수
 *
 * 5.0부터 ASC->GiveAbility()가 BlueprintCallable이므로,
 * 이 함수는 유지보수 목적 외에는 사용을 권장하지 않습니다.
 *
 * @param Ability 부여할 Gameplay Ability 클래스
 * @param bRemoveAfterActivation Ability 활성화 후 자동 제거 여부
 * @return 부여된 Ability의 핸들 (FGameplayAbilitySpecHandle)
 */
FGameplayAbilitySpecHandle UGSCAbilitySystemComponent::GrantAbility(const TSubclassOf<UGameplayAbility> Ability, const bool bRemoveAfterActivation)
{
	FGameplayAbilitySpecHandle AbilityHandle;
	// 서버 권한이 아닐 경우 에러 처리
	if (!IsOwnerActorAuthoritative())
	{
		GSC_LOG(Error, TEXT("GrantAbility가 서버 권한이 아닌 곳에서 호출되었습니다."));
		return AbilityHandle;
	}

	if (Ability)
	{
		// AbilitySpec 생성 후 RemoveAfterActivation 설정
		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.RemoveAfterActivation = bRemoveAfterActivation;

		// ASC->GiveAbility 호출
		AbilityHandle = GiveAbility(AbilitySpec);
	}
	return AbilityHandle;
}

/**
 * @brief 특정 Ability Set을 ASC에 부여하는 함수
 *
 * @param InAbilitySet 부여할 Ability Set
 * @param OutHandle 추후 제거 시 사용할 AbilitySet 핸들
 * @return 부여 성공 시 true, 실패 시 false
 */
bool UGSCAbilitySystemComponent::GiveAbilitySet(const UGSCAbilitySet* InAbilitySet, FGSCAbilitySetHandle& OutHandle)
{
	GSC_WLOG(Verbose,
	         TEXT("Ability Set \"%s\" 부여 시도 (Owner: %s, Avatar: %s)"),
	         *GetNameSafe(InAbilitySet),
	         *GetNameSafe(GetOwnerActor()),
	         *GetNameSafe(GetAvatarActor_Direct())
	);

	if (!InAbilitySet)
	{
		GSC_PLOG(Error, TEXT("유효하지 않은 Ability Set입니다."));
		return false;
	}

	// 실제로 AbilitySet::GrantToAbilitySystem 호출
	FText ErrorText;
	if (!InAbilitySet->GrantToAbilitySystem(this, OutHandle, &ErrorText))
	{
		GSC_PLOG(Error, TEXT("Ability Set %s 부여 중 오류 - %s"), *GetNameSafe(InAbilitySet), *ErrorText.ToString());
		return false;
	}

	return true;
}

/**
 * @brief ASC에서 특정 Ability Set을 제거하는 함수
 *
 * @param InAbilitySetHandle 제거할 Ability Set의 핸들
 * @return 제거 성공 시 true, 실패 시 false
 */
bool UGSCAbilitySystemComponent::ClearAbilitySet(FGSCAbilitySetHandle& InAbilitySetHandle)
{
	if (!InAbilitySetHandle.IsValid())
	{
		GSC_PLOG(Error, TEXT("유효하지 않은 Ability Set 핸들입니다. AbilitySetPathName이 설정되지 않았습니다."));
		return false;
	}

	FText ErrorText;
	if (!UGSCAbilitySet::RemoveFromAbilitySystem(this, InAbilitySetHandle, &ErrorText))
	{
		GSC_PLOG(Error, TEXT("Ability Set %s 제거 중 오류 - %s"), *InAbilitySetHandle.AbilitySetPathName, *ErrorText.ToString());
		return false;
	}

	return true;
}

/**
 * @brief Ability가 활성화되었을 때(성공적으로 시작되었을 때) 호출되는 콜백 함수
 *
 * @param Ability 활성화된 Ability의 포인터
 */
void UGSCAbilitySystemComponent::OnAbilityActivatedCallback(UGameplayAbility* Ability)
{
	GSC_LOG(Log, TEXT("Ability 활성화 콜백: %s"), *Ability->GetName());
	const AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		GSC_LOG(Error, TEXT("OwnerActor가 유효하지 않아 Ability: %s 활성화 처리를 진행할 수 없습니다."), *Ability->GetName());
		return;
	}

	// 만약 Companion Core Component가 있다면, 이벤트 브로드캐스트
	const UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	if (CoreComponent)
	{
		CoreComponent->OnAbilityActivated.Broadcast(Ability);
	}
}

/**
 * @brief Ability 활성화가 실패했을 때 호출되는 콜백 함수
 *
 * @param Ability 실패한 Ability 포인터
 * @param Tags 실패 사유를 나타내는 GameplayTag 컨테이너
 */
void UGSCAbilitySystemComponent::OnAbilityFailedCallback(const UGameplayAbility* Ability, const FGameplayTagContainer& Tags)
{
	GSC_LOG(Log, TEXT("Ability 활성화 실패 콜백: %s"), *Ability->GetName());

	const AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		GSC_LOG(Warning,
		        TEXT("OwnerActor가 유효하지 않아 Ability: %s 실패 처리를 진행할 수 없습니다. 태그: %s"),
		        *Ability->GetName(),
		        *Tags.ToString()
		);
		return;
	}

	const UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Avatar);

	// 실패 이벤트 브로드캐스트
	if (CoreComponent)
	{
		CoreComponent->OnAbilityFailed.Broadcast(Ability, Tags);
	}
	// AbilityQueueComponent가 있을 경우 큐 처리
	if (AbilityQueueComponent)
	{
		AbilityQueueComponent->OnAbilityFailed(Ability, Tags);
	}
}

/**
 * @brief Ability가 종료(End)되었을 때 호출되는 콜백 함수
 *
 * @param Ability 종료된 Ability 포인터
 */
void UGSCAbilitySystemComponent::OnAbilityEndedCallback(UGameplayAbility* Ability)
{
	GSC_LOG(Log, TEXT("Ability 종료 콜백: %s"), *Ability->GetName());
	const AActor* Avatar = GetAvatarActor();
	if (!Avatar)
	{
		GSC_LOG(Warning, TEXT("OwnerActor가 유효하지 않아 Ability: %s 종료 처리를 진행할 수 없습니다."), *Ability->GetName());
		return;
	}

	const UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(Avatar);
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Avatar);

	if (CoreComponent)
	{
		CoreComponent->OnAbilityEnded.Broadcast(Ability);
	}
	if (AbilityQueueComponent)
	{
		AbilityQueueComponent->OnAbilityEnded(Ability);
	}
}

/**
 * @brief 특정 Ability를 재부여할지 여부를 판단하는 함수
 *
 * @param InAbility 부여하려는 Ability 클래스
 * @param InLevel Ability 레벨
 * @return 재부여가 필요한 경우 true, 이미 존재하면 false
 */
bool UGSCAbilitySystemComponent::ShouldGrantAbility(const TSubclassOf<UGameplayAbility> InAbility, const int32 InLevel)
{
	// 만약 bResetAbilitiesOnSpawn이 true라면 항상 재부여를 시도
	if (bResetAbilitiesOnSpawn)
	{
		return true;
	}

	// 재부여 불필요 여부 확인
	// 현재 ASC에 이미 동일 Ability/레벨이 존재하면 재부여 생략
	TArray<FGameplayAbilitySpec> AbilitySpecs = GetActivatableAbilities();
	for (const FGameplayAbilitySpec& ActivatableAbility : AbilitySpecs)
	{
		if (!ActivatableAbility.Ability)
		{
			continue;
		}

		if (ActivatableAbility.Ability->GetClass() == InAbility && ActivatableAbility.Level == InLevel)
		{
			return false;
		}
	}
	return true;
}

/**
 * @brief 특정 Ability Set을 재부여할지 여부를 판단하는 함수
 *
 * @param InAbilitySet 부여하려는 Ability Set
 * @return 재부여가 필요한 경우 true, 이미 존재하면 false
 */
bool UGSCAbilitySystemComponent::ShouldGrantAbilitySet(const UGSCAbilitySet* InAbilitySet) const
{
	check(InAbilitySet);

	// ASC가 PlayerState에 있을 경우, 입력 바인딩 재설정 등을 위해 무조건 재부여
	if (IsPlayerStateOwner())
	{
		return true;
	}

	// 이미 AddedAbilitySets에 동일한 PathName이 있으면 재부여 생략
	for (const FGSCAbilitySetHandle& Handle : AddedAbilitySets)
	{
		if (Handle.AbilitySetPathName == InAbilitySet->GetPathName())
		{
			return false;
		}
	}
	return true;
}

/**
 * @brief 현재 ASC 소유자 액터가 PlayerState인지 여부를 반환
 *
 * @return PlayerState 타입이면 true, 아니면 false
 */
bool UGSCAbilitySystemComponent::IsPlayerStateOwner() const
{
	const AActor* LocalOwnerActor = GetOwnerActor();
	return LocalOwnerActor && LocalOwnerActor->IsA<APlayerState>();
}

void UGSCAbilitySystemComponent::GrantDefaultAbilitiesAndAttributes(AActor* InOwnerActor, AActor* InAvatarActor)
{
	GSC_WLOG(Verbose, TEXT("소유자: %s, 아바타: %s"), *GetNameSafe(InOwnerActor), *GetNameSafe(InAvatarActor))

	// 1) bResetAttributesOnSpawn이 true면, 이전에 부여된 AttributeSet 제거
	if (bResetAttributesOnSpawn)
	{
		for (UAttributeSet* AttributeSet : AddedAttributes)
		{
#if ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 1
			RemoveSpawnedAttribute(AttributeSet);
#else
			GetSpawnedAttributes_Mutable().Remove(AttributeSet);
#endif
		}
		AddedAttributes.Empty(GrantedAttributes.Num());
	}

	// 2) bResetAbilitiesOnSpawn이 true면, 이전에 부여된 Ability를 제거하고 Delegate도 해제
	if (bResetAbilitiesOnSpawn)
	{
		for (const FGSCMappedAbility& DefaultAbilityHandle : AddedAbilityHandles)
		{
			SetRemoveAbilityOnEnd(DefaultAbilityHandle.Handle);
		}

		for (FDelegateHandle InputBindingDelegateHandle : InputBindingDelegateHandles)
		{
			OnGiveAbilityDelegate.Remove(InputBindingDelegateHandle);
			InputBindingDelegateHandle.Reset();
		}

		AddedAbilityHandles.Empty(GrantedAbilities.Num());
		InputBindingDelegateHandles.Empty();
	}

	// 3) 아바타 액터가 있으면 입력 바인딩 컴포넌트를 찾음
	UGSCAbilityInputBindingComponent* InputComponent = IsValid(InAvatarActor)
		                                                   ? InAvatarActor->FindComponentByClass<UGSCAbilityInputBindingComponent>()
		                                                   : nullptr;

	// 4) GrantedAbilities 목록을 순회하며 Ability 부여 (서버 권한에서만)
	for (const FGSCAbilityInputMapping& GrantedAbility : GrantedAbilities)
	{
		const TSubclassOf<UGameplayAbility> Ability = GrantedAbility.Ability;
		UInputAction* InputAction = GrantedAbility.InputAction;

		if (!Ability)
			continue;

		FGameplayAbilitySpec NewAbilitySpec = BuildAbilitySpecFromClass(Ability, GrantedAbility.Level);

		if (IsOwnerActorAuthoritative() && ShouldGrantAbility(Ability, GrantedAbility.Level))
		{
			GSC_LOG(Log, TEXT("Authority에서 Ability 부여 시도: %s"), *NewAbilitySpec.Ability->GetClass()->GetName());
			FGameplayAbilitySpecHandle AbilityHandle = GiveAbility(NewAbilitySpec);
			AddedAbilityHandles.Add(FGSCMappedAbility(AbilityHandle, NewAbilitySpec, InputAction));
		}

		// 클라이언트 또는 스탠드얼론 환경에서 InputAction이 설정된 경우 입력 바인딩 처리
		if (InputComponent && InputAction)
		{
			if (const FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromClass(Ability))
			{
				// 이미 Spec이 있다면 바로 바인딩
				InputComponent->SetInputBinding(InputAction, GrantedAbility.TriggerEvent, AbilitySpec->Handle);
			}
			else
			{
				// 아직 Spec이 없으면 OnGiveAbilityDelegate를 통해 부여 시점에 바인딩
				FDelegateHandle DelegateHandle = OnGiveAbilityDelegate.AddUObject(
					this,
					&UGSCAbilitySystemComponent::HandleOnGiveAbility,
					InputComponent,
					InputAction,
					GrantedAbility.TriggerEvent,
					NewAbilitySpec
				);
				InputBindingDelegateHandles.Add(DelegateHandle);
			}
		}
	}

	// 5) GrantedAttributes 목록을 순회하며 AttributeSet 부여
	for (const FGSCAttributeSetDefinition& AttributeSetDefinition : GrantedAttributes)
	{
		if (AttributeSetDefinition.AttributeSet)
		{
			const bool bHasAttributeSet = GetAttributeSubobject(AttributeSetDefinition.AttributeSet) != nullptr;
			GSC_LOG(Verbose,
			        TEXT("AttributeSet(%s) 이미 존재 여부: %s"),
			        *GetNameSafe(AttributeSetDefinition.AttributeSet),
			        bHasAttributeSet ? TEXT("True") : TEXT("False")
			);

			// 없는 경우에만 부여
			if (!bHasAttributeSet && InOwnerActor)
			{
				UAttributeSet* AttributeSet = NewObject<UAttributeSet>(InOwnerActor, AttributeSetDefinition.AttributeSet);
				// 필요한 경우 DataTable 초기화
				if (AttributeSetDefinition.InitializationData)
				{
					AttributeSet->InitFromMetaDataTable(AttributeSetDefinition.InitializationData);
				}
				AddedAttributes.Add(AttributeSet);
				AddAttributeSetSubobject(AttributeSet);
			}
		}
	}
}

void UGSCAbilitySystemComponent::GrantDefaultAbilitySets(AActor* InOwnerActor, AActor* InAvatarActor)
{
	GSC_WLOG(Verbose,
	         TEXT("Ability Set 초기화 시도 - 소유자: %s, 아바타: %s"),
	         *GetNameSafe(InOwnerActor),
	         *GetNameSafe(InAvatarActor)
	);

	if (!IsValid(InOwnerActor) || !IsValid(InAvatarActor))
	{
		return;
	}

	// GrantedAbilitySets 목록 순회
	for (const TSoftObjectPtr<UGSCAbilitySet>& AbilitySetEntry : GrantedAbilitySets)
	{
		if (const UGSCAbilitySet* AbilitySet = AbilitySetEntry.LoadSynchronous())
		{
			// 재부여 여부 검사
			if (!ShouldGrantAbilitySet(AbilitySet))
			{
				continue;
			}

			// AbilitySet이 입력 바인딩이 필요한 경우, 아바타 액터에 UGSCAbilityInputBindingComponent 확인
			if (AbilitySet->HasInputBinding())
			{
				// PlayerState 단계에서 AvatarActor가 PlayerState일 수 있으므로 Pawn인지 확인
				const APawn* AvatarPawn = Cast<APawn>(InAvatarActor);
				if (!AvatarPawn)
				{
					// 아직 Pawn이 아니라면(예: PlayerState 초기화), 추후 재시도
					return;
				}

				const UGSCAbilityInputBindingComponent* InputBindingComponent = AvatarPawn->FindComponentByClass<UGSCAbilityInputBindingComponent>();
				if (!InputBindingComponent)
				{
					const FText FormatText = NSLOCTEXT(
						"GSCAbilitySystemComponent",
						"Error_AbilitySet_Invalid_InputBindingComponent",
						"The set contains Abilities with Input bindings but {0} is missing the required UGSCAbilityInputBindingComponent actor component."
					);

					const FText ErrorText = FText::Format(FormatText, FText::FromString(AvatarPawn->GetName()));
					GSC_PLOG(Error,
					         TEXT("AbilitySet %s 부여 중 오류 - %s"),
					         *GetNameSafe(AbilitySet),
					         *ErrorText.ToString()
					);
					return;
				}
			}

			// AbilitySet 부여
			FText ErrorText;
			FGSCAbilitySetHandle Handle;
			if (!AbilitySet->GrantToAbilitySystem(this, Handle, &ErrorText, false))
			{
				GSC_PLOG(Error,
				         TEXT("AbilitySet %s 부여 중 오류 - %s"),
				         *GetNameSafe(AbilitySet),
				         *ErrorText.ToString()
				);
				continue;
			}

			// 성공적으로 부여된 Set Handle 캐시
			AddedAbilitySets.AddUnique(Handle);
		}
	}
}

void UGSCAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	// AbilitySpec 디버그 정보 로그
	GSC_WLOG(Verbose, TEXT("%s"), *AbilitySpec.GetDebugString());

	// 멀티캐스트 델리게이트 호출
	OnGiveAbilityDelegate.Broadcast(AbilitySpec);
}

void UGSCAbilitySystemComponent::GrantStartupEffects()
{
	// 서버 권한이 아닌 경우는 처리 생략
	if (!IsOwnerActorAuthoritative())
	{
		return;
	}

	// 이전에 부여된 Effect를 제거
	for (const FActiveGameplayEffectHandle AddedEffect : AddedEffects)
	{
		RemoveActiveGameplayEffect(AddedEffect);
	}
	AddedEffects.Empty(GrantedEffects.Num());

	// EffectContext 생성 (소스 객체를 ASC 자신으로 지정)
	FGameplayEffectContextHandle EffectContext = MakeEffectContext();
	EffectContext.AddSourceObject(this);

	// GrantedEffects에 정의된 각 Effect를 부여
	for (const TSubclassOf<UGameplayEffect>& GameplayEffect : GrantedEffects)
	{
		FGameplayEffectSpecHandle NewHandle = MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
		if (NewHandle.IsValid())
		{
			FActiveGameplayEffectHandle EffectHandle = ApplyGameplayEffectSpecToTarget(*NewHandle.Data.Get(), this);
			AddedEffects.Add(EffectHandle);
		}
	}
}

/**
 * @brief Pawn 컨트롤러가 변경되었을 때 호출되는 함수
 *
 * @param Pawn 해당 Pawn
 * @param NewController 새로 할당된 컨트롤러
 *
 * Pawn이 소유자를 교체할 때(예: PossessedBy), AbilityActorInfo의 PlayerController를 재설정합니다.
 */
void UGSCAbilitySystemComponent::OnPawnControllerChanged(APawn* Pawn, AController* NewController)
{
	// Pawn == 소유자이고, 새 컨트롤러가 기존과 다른 경우에만 재초기화
	if (AbilityActorInfo && AbilityActorInfo->OwnerActor == Pawn && AbilityActorInfo->PlayerController != NewController)
	{
		// NewController가 null이면 재초기화 불필요 (TargetActor EndPlay 등 상황에서)
		if (!NewController)
		{
			return;
		}

		// 기존 OwnerActor와 AvatarActor로 AbilityActorInfo 재설정
		AbilityActorInfo->InitFromActor(AbilityActorInfo->OwnerActor.Get(), AbilityActorInfo->AvatarActor.Get(), this);
	}
}

void UGSCAbilitySystemComponent::HandleOnGiveAbility(FGameplayAbilitySpec& AbilitySpec, UGSCAbilityInputBindingComponent* InputComponent,
                                                     UInputAction* InputAction, EGSCAbilityTriggerEvent TriggerEvent, FGameplayAbilitySpec NewAbilitySpec)
{
	// 디버그 로그 출력 (AbilitySpec과 관련된 입력 액션, 트리거 이벤트 확인)
	GSC_LOG(
		Log,
		TEXT("UGSCAbilitySystemComponent::HandleOnGiveAbility: %s, Ability: %s, 입력: %s (TriggerEvent: %s), (InputComponent: %s)"),
		*AbilitySpec.Handle.ToString(),
		*GetNameSafe(AbilitySpec.Ability),
		*GetNameSafe(InputAction),
		*UEnum::GetValueAsName(TriggerEvent).ToString(),
		*GetNameSafe(InputComponent)
	);

	// 만약 InputComponent, InputAction이 유효하고, 실제로 동일 AbilitySpec인 경우 입력 바인딩 수행
	if (InputComponent && InputAction && AbilitySpec.Ability == NewAbilitySpec.Ability)
	{
		InputComponent->SetInputBinding(InputAction, TriggerEvent, AbilitySpec.Handle);
	}
}
