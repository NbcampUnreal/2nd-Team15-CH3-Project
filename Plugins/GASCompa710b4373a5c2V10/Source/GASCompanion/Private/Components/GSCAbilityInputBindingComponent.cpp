#include "Components/GSCAbilityInputBindingComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GSCLog.h"

namespace GSCAbilityInputBindingComponent_Impl
{
	// 유효하지 않은 InputID 값 (0)
	constexpr int32 InvalidInputID = 0;

	// 증가하는 InputID 값 (전역 변수)
	int32 IncrementingInputID = InvalidInputID;

	/**
	 * @brief 다음 유효한 InputID 값을 반환하는 함수
	 * @return 증가된 InputID 값
	 */
	static int32 GetNextInputID()
	{
		// IncrementingInputID를 증가시켜서 반환합니다.
		return ++IncrementingInputID;
	}
}

void UGSCAbilityInputBindingComponent::SetupPlayerControls_Implementation(UEnhancedInputComponent* PlayerInputComponent)
{
	// 기존 바인딩들을 초기화합니다.
	ResetBindings();

	// 디버그 로그: 현재 매핑된 능력 개수를 출력 (로그 메시지를 한글로 변경)
	GSC_WLOG(Verbose, TEXT("플레이어 컨트롤 설정 시작 (MappedAbilities: %d)"), MappedAbilities.Num())

	// 매핑된 각 입력 액션에 대해 반복 처리
	for (const auto& Ability : MappedAbilities)
	{
		UInputAction* InputAction = Ability.Key;
		const FGSCAbilityInputBinding AbilityInputBinding = Ability.Value;

		const ETriggerEvent TriggerEvent = GetInputActionTriggerEvent(AbilityInputBinding.TriggerEvent);

		GSC_PLOG(Log, TEXT("입력 액션 '%s'의 pressed 핸들 설정 (트리거: %s)"), *GetNameSafe(InputAction), *UEnum::GetValueAsName(TriggerEvent).ToString())

		// 입력 액션의 Pressed 이벤트 바인딩: InputComponent에 해당 액션을 바인딩하고 핸들을 획득
		const uint32 PressedHandle = InputComponent->BindAction(InputAction, TriggerEvent, this, &UGSCAbilityInputBindingComponent::OnAbilityInputPressed, InputAction).GetHandle();

		// 중복 등록을 방지하기 위해 핸들을 저장
		RegisteredInputHandles.AddUnique(PressedHandle);

		// 입력 액션의 Released 이벤트 바인딩: 완료(Completed) 이벤트로 바인딩
		const uint32 ReleasedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UGSCAbilityInputBindingComponent::OnAbilityInputReleased, InputAction).GetHandle();
		RegisteredInputHandles.AddUnique(ReleasedHandle);
	}

	if (TargetInputConfirm)
	{
		const ETriggerEvent TriggerEvent = GetInputActionTriggerEvent(TargetConfirmTriggerEvent);
		OnConfirmHandle = InputComponent->BindAction(TargetInputConfirm, TriggerEvent, this, &UGSCAbilityInputBindingComponent::OnLocalInputConfirm).GetHandle();
	}

	if (TargetInputCancel)
	{
		const ETriggerEvent TriggerEvent = GetInputActionTriggerEvent(TargetCancelTriggerEvent);
		OnCancelHandle = InputComponent->BindAction(TargetInputCancel, TriggerEvent, this, &UGSCAbilityInputBindingComponent::OnLocalInputCancel).GetHandle();
	}

	// 능력 시스템 관련 추가 설정 실행
	RunAbilitySystemSetup();
}

void UGSCAbilityInputBindingComponent::ReleaseInputComponent(AController* OldController)
{
	ResetBindings();

	Super::ReleaseInputComponent();
}

FGameplayAbilitySpecHandle UGSCAbilityInputBindingComponent::GiveAbilityWithInput(const TSubclassOf<UGameplayAbility> Ability, const int32 Level, UInputAction* InputAction, const EGSCAbilityTriggerEvent TriggerEvent)
{
	GSC_WLOG(
		Verbose,
		TEXT("능력 '%s' (레벨: %d)을 부여합니다. 입력: %s, 트리거: %s"),
		*GetNameSafe(Ability),
		Level,
		*GetNameSafe(InputAction),
		*UEnum::GetValueAsString(TriggerEvent)
	);

	// 소유자에서 Ability System Component(ASC)를 가져옵니다.
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (!ASC)
	{
		GSC_PLOG(Error, TEXT("액터 '%s'에서 Ability System Component를 찾을 수 없습니다."), *GetNameSafe(GetOwner()));
		return FGameplayAbilitySpecHandle();
	}

	// AbilitySpec을 생성 및 초기화 (클래스, 레벨 기반)
	const FGameplayAbilitySpec AbilitySpec = ASC->BuildAbilitySpecFromClass(Ability, Level);

	if (!IsValid(AbilitySpec.Ability))
	{
		GSC_PLOG(Error, TEXT("유효하지 않은 Ability Class가 전달되었습니다."));
		return FGameplayAbilitySpecHandle();
	}

	if (!ASC->IsOwnerActorAuthoritative())
	{
		GSC_PLOG(Error, TEXT("클라이언트에서 '%s' 능력을 호출했습니다. 서버 권한에서 실행되어야 합니다."), *AbilitySpec.Ability->GetName());
		return FGameplayAbilitySpecHandle();
	}

	// ASC를 통해 능력을 부여 (검증 및 권한 체크 포함)
	const FGameplayAbilitySpecHandle AbilityHandle = ASC->GiveAbility(AbilitySpec);

	// 능력 부여 후, 입력 액션이 제공되면 클라이언트에서 입력 바인딩을 설정하도록 RPC 호출
	if (AbilityHandle.IsValid() && InputAction)
	{
		ClientBindInput(InputAction, TriggerEvent, AbilityHandle);
	}

	return AbilityHandle;
}

void UGSCAbilityInputBindingComponent::ClientBindInput_Implementation(UInputAction* InInputAction, const EGSCAbilityTriggerEvent InTriggerEvent, const FGameplayAbilitySpecHandle& InAbilityHandle)
{
	// 실제 입력 바인딩을 설정하는 함수 호출
	SetInputBinding(InInputAction, InTriggerEvent, InAbilityHandle);
}

void UGSCAbilityInputBindingComponent::SetInputBinding(UInputAction* InputAction, const EGSCAbilityTriggerEvent TriggerEvent, const FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace GSCAbilityInputBindingComponent_Impl;

	// ASC 내에서 해당 능력 스펙을 검색
	FGameplayAbilitySpec* BindingAbility = FindAbilitySpec(AbilityHandle);

	// 기존에 해당 입력 액션에 바인딩된 정보가 있는지 확인
	FGSCAbilityInputBinding* AbilityInputBinding = MappedAbilities.Find(InputAction);
	if (AbilityInputBinding)
	{
		// 기존 바인딩된 능력의 InputID를 초기화하여 중복 사용 방지
		FGameplayAbilitySpec* OldBoundAbility = FindAbilitySpec(AbilityInputBinding->BoundAbilitiesStack.Top());
		if (OldBoundAbility && OldBoundAbility->InputID == AbilityInputBinding->InputID)
		{
			OldBoundAbility->InputID = InvalidInputID;
		}
	}
	else
	{
		// 바인딩 정보가 없으면 새로 추가하고, 새로운 InputID와 트리거 이벤트 타입을 설정
		AbilityInputBinding = &MappedAbilities.Add(TObjectPtr<UInputAction>(InputAction));
		AbilityInputBinding->InputID = GetNextInputID();
		AbilityInputBinding->TriggerEvent = TriggerEvent;
	}

	// 만약 해당 능력 스펙이 이미 존재하면, 그 InputID를 갱신합니다.
	if (BindingAbility)
	{
		BindingAbility->InputID = AbilityInputBinding->InputID;
	}

	// 능력 핸들을 해당 입력 액션의 바인딩 스택에 추가합니다.
	AbilityInputBinding->BoundAbilitiesStack.Push(AbilityHandle);
	// 입력 액션에 대해 실제 이벤트 바인딩을 시도합니다.
	TryBindAbilityInput(InputAction, *AbilityInputBinding);
}

void UGSCAbilityInputBindingComponent::ClearInputBinding(const FGameplayAbilitySpecHandle AbilityHandle)
{
	using namespace GSCAbilityInputBindingComponent_Impl;

	// ASC 내에서 해당 능력 스펙을 검색
	FGameplayAbilitySpec* FoundAbility = FindAbilitySpec(AbilityHandle);
	if (!FoundAbility)
	{
		return;
	}

	// 해당 능력의 InputID와 일치하는 매핑 정보를 찾기 위해 반복합니다.
	auto MappedIterator = MappedAbilities.CreateIterator();
	while (MappedIterator)
	{
		if (MappedIterator.Value().InputID == FoundAbility->InputID)
		{
			break;
		}
		++MappedIterator;
	}

	if (MappedIterator)
	{
		// 찾은 매핑 정보를 사용하여, 스택에서 해당 능력 핸들을 제거
		FGSCAbilityInputBinding& AbilityInputBinding = MappedIterator.Value();

		if (AbilityInputBinding.BoundAbilitiesStack.Remove(AbilityHandle) > 0)
		{
			// 스택에 다른 능력이 남아 있다면, 최신 능력에 InputID를 할당
			if (AbilityInputBinding.BoundAbilitiesStack.Num() > 0)
			{
				FGameplayAbilitySpec* StackedAbility = FindAbilitySpec(AbilityInputBinding.BoundAbilitiesStack.Top());
				if (StackedAbility && StackedAbility->InputID == 0)
				{
					StackedAbility->InputID = AbilityInputBinding.InputID;
				}
			}
			else
			{
				// 스택이 비었으면, 해당 입력 액션의 매핑 정보를 제거 (주의: Iterator 이후 사용 주의)
				RemoveEntry(MappedIterator.Key());
			}
			// 이후 AbilityInputBinding을 참조하면 안 됨
			FoundAbility->InputID = InvalidInputID;
		}
	}
}

void UGSCAbilityInputBindingComponent::ClearAbilityBindings(UInputAction* InputAction)
{
	// 내부 함수 RemoveEntry()를 호출하여 해당 입력 액션의 매핑 정보를 제거합니다.
	RemoveEntry(InputAction);
}

UInputAction* UGSCAbilityInputBindingComponent::GetBoundInputActionForAbility(const UGameplayAbility* Ability)
{
	if (!Ability)
	{
		GSC_WLOG(Error, TEXT("전달된 Ability가 nullptr입니다."));
		return nullptr;
	}

	// Ability가 인스턴스화되어 있고 현재 액터 정보가 있다면, 해당 ASC를 사용; 그렇지 않으면 소유자에서 ASC를 가져옴.
	UAbilitySystemComponent* AbilitySystemComponent = Ability->IsInstantiated() && Ability->GetCurrentActorInfo() ? Ability->GetAbilitySystemComponentFromActorInfo() : UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());

	if (!AbilitySystemComponent)
	{
		GSC_WLOG(Error, TEXT("'%s' Ability의 입력 액션을 찾으려 했으나, AbilitySystemComponent를 찾을 수 없습니다."), *GetNameSafe(Ability))
		return nullptr;
	}

	// ASC 내에 저장된 능력 스펙의 InputID를 업데이트합니다.
	UpdateAbilitySystemBindings(AbilitySystemComponent);

	// Ability 클래스에 대한 능력 스펙을 검색합니다.
	const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(Ability->GetClass());
	if (!AbilitySpec)
	{
		GSC_WLOG(Error, TEXT("AbilitySystemComponent가 '%s' Ability Spec을 반환하지 않습니다."), *GetNameSafe(Ability->GetClass()))
		return nullptr;
	}

	// 매핑된 입력 액션을 반환합니다.
	return GetBoundInputActionForAbilitySpec(AbilitySpec);
}

// ReSharper disable once CppPassValueParameterByConstReference
UInputAction* UGSCAbilityInputBindingComponent::GetBoundInputActionForAbilityClass(TSubclassOf<UGameplayAbility> InAbilityClass)
{
	if (!InAbilityClass)
	{
		GSC_WLOG(Error, TEXT("전달된 Ability 클래스가 유효하지 않습니다."));
		return nullptr;
	}

	// 기본 객체를 사용하여 입력 액션을 검색합니다.
	return GetBoundInputActionForAbility(InAbilityClass.GetDefaultObject());
}

UInputAction* UGSCAbilityInputBindingComponent::GetBoundInputActionForAbilitySpec(const FGameplayAbilitySpec* AbilitySpec) const
{
	// AbilitySpec가 유효한지 확인
	check(AbilitySpec);

	UInputAction* FoundInputAction = nullptr;

	// 저장된 모든 매핑 정보를 순회하며, AbilitySpec의 InputID와 일치하는 입력 액션을 찾습니다.
	for (const TPair<TObjectPtr<UInputAction>, FGSCAbilityInputBinding>& MappedAbility : MappedAbilities)
	{
		const FGSCAbilityInputBinding AbilityInputBinding = MappedAbility.Value;

		if (AbilityInputBinding.InputID == AbilitySpec->InputID)
		{
			FoundInputAction = MappedAbility.Key;
			break;
		}
	}

	return FoundInputAction;
}

void UGSCAbilityInputBindingComponent::ResetBindings()
{
	// 모든 매핑 정보에 대해 반복
	for (const TPair<TObjectPtr<UInputAction>, FGSCAbilityInputBinding>& InputBinding : MappedAbilities)
	{
		if (AbilityComponent)
		{
			// 현재 입력 액션에 할당된 InputID를 저장
			const int32 ExpectedInputID = InputBinding.Value.InputID;

			// 연결된 모든 능력 스펙에 대해, InputID가 일치하면 초기화
			for (const FGameplayAbilitySpecHandle AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
			{
				FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
				if (FoundAbility && FoundAbility->InputID == ExpectedInputID)
				{
					FoundAbility->InputID = GSCAbilityInputBindingComponent_Impl::InvalidInputID;
				}
			}
		}
	}

	// 입력 컴포넌트가 유효하면, 타겟 확인/취소 및 등록된 모든 입력 핸들을 제거
	if (InputComponent)
	{
		InputComponent->RemoveBindingByHandle(OnConfirmHandle);
		InputComponent->RemoveBindingByHandle(OnCancelHandle);

		// 저장된 모든 입력 핸들을 반복하며 제거
		for (const uint32 InputHandle : RegisteredInputHandles)
		{
			InputComponent->RemoveBindingByHandle(InputHandle);
		}

		// 입력 핸들 리스트 초기화
		RegisteredInputHandles.Reset();
	}

	// ASC 참조 초기화
	AbilityComponent = nullptr;
}

void UGSCAbilityInputBindingComponent::RunAbilitySystemSetup()
{
	const AActor* MyOwner = GetOwner();
	check(MyOwner);

	// 소유자 액터에서 Ability System Component를 가져옵니다.
	AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(MyOwner);
	if (!AbilityComponent)
		return;

	// 저장된 모든 매핑 정보를 순회하며 새로운 InputID를 할당
	for (auto& InputBinding : MappedAbilities)
	{
		const int32 NewInputID = GSCAbilityInputBindingComponent_Impl::GetNextInputID();
		InputBinding.Value.InputID = NewInputID;

		// 연결된 각 능력 스펙에 대해 새로운 InputID를 적용
		for (const FGameplayAbilitySpecHandle AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
		{
			FGameplayAbilitySpec* FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(AbilityHandle);
			if (FoundAbility != nullptr)
			{
				FoundAbility->InputID = NewInputID;
			}
		}
	}
}

void UGSCAbilityInputBindingComponent::UpdateAbilitySystemBindings(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
	{
		GSC_LOG(Error, TEXT("UGSCAbilityInputBindingComponent::UpdateAbilitySystemBindings - 전달된 ASC가 유효하지 않습니다."));
		return;
	}

	// 저장된 모든 매핑 정보를 순회
	for (auto& InputBinding : MappedAbilities)
	{
		const int32 InputID = InputBinding.Value.InputID;
		if (InputID <= 0)
		{
			continue;
		}

		// 연결된 각 능력 스펙에 대해 InputID를 갱신
		for (const FGameplayAbilitySpecHandle AbilityHandle : InputBinding.Value.BoundAbilitiesStack)
		{
			FGameplayAbilitySpec* FoundAbility = AbilitySystemComponent->FindAbilitySpecFromHandle(AbilityHandle);
			if (FoundAbility != nullptr)
			{
				FoundAbility->InputID = InputID;
			}
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UGSCAbilityInputBindingComponent::OnAbilityInputPressed(UInputAction* InputAction)
{
	// 최초 바인딩 시 ASC가 유효하지 않을 수 있으므로, ASC를 업데이트하거나 재설정 시도
	if (AbilityComponent)
	{
		UpdateAbilitySystemBindings(AbilityComponent);
	}
	else
	{
		RunAbilitySystemSetup();
	}

	// ASC가 유효하면, 해당 입력 액션과 연결된 능력의 InputID를 통해 LocalInputPressed()를 호출
	if (AbilityComponent)
	{
		using namespace GSCAbilityInputBindingComponent_Impl;

		const FGSCAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
		if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
		{
			AbilityComponent->AbilityLocalInputPressed(FoundBinding->InputID);
		}
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UGSCAbilityInputBindingComponent::OnAbilityInputReleased(UInputAction* InputAction)
{
	// The AbilitySystemComponent may need to have specs inputID updated here for clients... try again.
	UpdateAbilitySystemBindings(AbilityComponent);

	if (AbilityComponent)
	{
		using namespace GSCAbilityInputBindingComponent_Impl;

		const FGSCAbilityInputBinding* FoundBinding = MappedAbilities.Find(InputAction);
		if (FoundBinding && ensure(FoundBinding->InputID != InvalidInputID))
		{
			AbilityComponent->AbilityLocalInputReleased(FoundBinding->InputID);
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGSCAbilityInputBindingComponent::OnLocalInputConfirm()
{
	GSC_LOG(Verbose, TEXT("UGSCAbilityInputBindingComponent::OnLocalInputConfirm 호출"))

	if (AbilityComponent)
	{
		AbilityComponent->LocalInputConfirm();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGSCAbilityInputBindingComponent::OnLocalInputCancel()
{
	GSC_LOG(Verbose, TEXT("UGSCAbilityInputBindingComponent::OnLocalInputCancel 호출"))
	if (AbilityComponent)
	{
		AbilityComponent->LocalInputCancel();
	}
}

void UGSCAbilityInputBindingComponent::RemoveEntry(const UInputAction* InputAction)
{
	if (FGSCAbilityInputBinding* Bindings = MappedAbilities.Find(InputAction))
	{
		// 입력 컴포넌트가 유효하면, Pressed와 Released 이벤트 바인딩을 제거합니다.
		if (InputComponent)
		{
			InputComponent->RemoveBindingByHandle(Bindings->OnPressedHandle);
			InputComponent->RemoveBindingByHandle(Bindings->OnReleasedHandle);
		}

		// 연결된 모든 능력 스펙에 대해, 현재 InputID와 일치하면 초기화
		for (const FGameplayAbilitySpecHandle AbilityHandle : Bindings->BoundAbilitiesStack)
		{
			using namespace GSCAbilityInputBindingComponent_Impl;

			FGameplayAbilitySpec* AbilitySpec = FindAbilitySpec(AbilityHandle);
			if (AbilitySpec && AbilitySpec->InputID == Bindings->InputID)
			{
				AbilitySpec->InputID = InvalidInputID;
			}
		}

		// 해당 입력 액션의 매핑 정보를 맵에서 제거
		MappedAbilities.Remove(InputAction);
	}
}

FGameplayAbilitySpec* UGSCAbilityInputBindingComponent::FindAbilitySpec(const FGameplayAbilitySpecHandle Handle) const
{
	FGameplayAbilitySpec* FoundAbility = nullptr;
	if (AbilityComponent)
	{
		FoundAbility = AbilityComponent->FindAbilitySpecFromHandle(Handle);
	}
	return FoundAbility;
}

void UGSCAbilityInputBindingComponent::TryBindAbilityInput(UInputAction* InputAction, FGSCAbilityInputBinding& AbilityInputBinding)
{
	// 디버그 로그: 입력 액션에 대해 바인딩을 시도 (한글 로그 메시지)
	GSC_WLOG(Verbose, TEXT("입력 액션 '%s'에 대한 컨트롤 설정 시도 (InputID: %d)"), *GetNameSafe(InputAction), AbilityInputBinding.InputID)

	if (InputComponent)
	{
		if (AbilityInputBinding.OnPressedHandle == 0)
		{
			const ETriggerEvent TriggerEvent = GetInputActionTriggerEvent(AbilityInputBinding.TriggerEvent);

			GSC_PLOG(Log, TEXT("입력 액션 '%s'의 pressed 핸들 바인딩 (트리거: %s)"), *GetNameSafe(InputAction), *UEnum::GetValueAsName(TriggerEvent).ToString())
			AbilityInputBinding.OnPressedHandle = InputComponent->BindAction(InputAction, TriggerEvent, this, &UGSCAbilityInputBindingComponent::OnAbilityInputPressed, InputAction).GetHandle();
			RegisteredInputHandles.AddUnique(AbilityInputBinding.OnPressedHandle);
		}

		// Released 이벤트가 아직 바인딩되지 않은 경우
		if (AbilityInputBinding.OnReleasedHandle == 0)
		{
			AbilityInputBinding.OnReleasedHandle = InputComponent->BindAction(InputAction, ETriggerEvent::Completed, this, &UGSCAbilityInputBindingComponent::OnAbilityInputReleased, InputAction).GetHandle();
			RegisteredInputHandles.AddUnique(AbilityInputBinding.OnReleasedHandle);
		}
	}
}

ETriggerEvent UGSCAbilityInputBindingComponent::GetInputActionTriggerEvent(const EGSCAbilityTriggerEvent TriggerEvent)
{
	// 내부 트리거 타입에 따라 실제 ETriggerEvent 값을 반환
	return TriggerEvent == EGSCAbilityTriggerEvent::Started ? ETriggerEvent::Started : TriggerEvent == EGSCAbilityTriggerEvent::Triggered ? ETriggerEvent::Triggered : ETriggerEvent::Started;
}
