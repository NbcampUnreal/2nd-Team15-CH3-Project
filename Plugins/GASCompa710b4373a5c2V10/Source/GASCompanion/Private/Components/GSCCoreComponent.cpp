#include "Components/GSCCoreComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GSCGameplayAbility.h"
#include "Abilities/Attributes/GSCAttributeSet.h"
#include "Core/Settings/GSCDeveloperSettings.h"
#include "GameFramework/Character.h"
#include "GSCLog.h"


UGSCCoreComponent::UGSCCoreComponent()
{
	// 게임 시작 시점에 초기화, 매 프레임 틱 비활성화
	PrimaryComponentTick.bCanEverTick = false;
	// 네트워크 복제 활성화
	SetIsReplicatedByDefault(true);
}

void UGSCCoreComponent::BeginPlay()
{
	Super::BeginPlay();

	// 소유 액터 및 ASC 설정
	SetupOwner();
}

void UGSCCoreComponent::BeginDestroy()
{
	// 소멸 시점에 ASC 델리게이트를 해제
	ShutdownAbilitySystemDelegates(OwnerAbilitySystemComponent);
	Super::BeginDestroy();
}

void UGSCCoreComponent::SetupOwner()
{
	// 소유 액터가 유효한지 확인
	if (!GetOwner())
		return;

	OwnerActor = GetOwner();
	if (!OwnerActor)
		return;

	// Pawn, Character 캐스팅
	OwnerPawn = Cast<APawn>(OwnerActor);
	OwnerCharacter = Cast<ACharacter>(OwnerActor);

	// AbilitySystemComponent를 가져옴
	OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
}

void UGSCCoreComponent::RegisterAbilitySystemDelegates(UAbilitySystemComponent* ASC)
{
	// 로그: "ASC에 델리게이트를 등록 중: %s"
	GSC_WLOG(Verbose, TEXT("ASC에 델리게이트를 등록 중: %s"), *GetNameSafe(ASC))

	if (!ASC)
	{
		return;
	}

	// 중복 등록을 방지하기 위해, 기존에 등록된 델리게이트를 먼저 해제
	ShutdownAbilitySystemDelegates(ASC);

	// ASC가 가진 모든 속성(Attribute)을 가져와서
	// OnAttributeChanged / OnDamageAttributeChanged에 바인딩
	TArray<FGameplayAttribute> Attributes;
	ASC->GetAllAttributes(Attributes);

	for (FGameplayAttribute Attribute : Attributes)
	{
		// 데미지 속성인지 판별 후 다른 콜백 바인딩
		if (Attribute == UGSCAttributeSet::GetDamageAttribute() || Attribute == UGSCAttributeSet::GetStaminaDamageAttribute())
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UGSCCoreComponent::OnDamageAttributeChanged);
		}
		else
		{
			ASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UGSCCoreComponent::OnAttributeChanged);
		}
	}

	// GameplayEffect 추가 이벤트 바인딩
	ASC->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(this, &UGSCCoreComponent::OnActiveGameplayEffectAdded);

	// GameplayEffect 제거 이벤트 바인딩
	ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &UGSCCoreComponent::OnAnyGameplayEffectRemoved);

	// GameplayTag가 추가/제거될 때 발생하는 이벤트 바인딩
	ASC->RegisterGenericGameplayTagEvent().AddUObject(this, &UGSCCoreComponent::OnAnyGameplayTagChanged);

	// Ability가 커밋(활성화 등)될 때 발생하는 이벤트 바인딩
	ASC->AbilityCommittedCallbacks.AddUObject(this, &UGSCCoreComponent::OnAbilityCommitted);
}

void UGSCCoreComponent::ShutdownAbilitySystemDelegates(UAbilitySystemComponent* ASC)
{
	// 로그: "UGSCCoreComponent::ShutdownAbilitySystemDelegates ASC: %s"
	GSC_LOG(Log, TEXT("UGSCCoreComponent::ShutdownAbilitySystemDelegates ASC: %s"), ASC ? *ASC->GetName() : TEXT("NONE"))

	if (!ASC)
		return;

	// ASC가 가진 모든 Attribute에 대해 바인딩 해제
	TArray<FGameplayAttribute> Attributes;
	ASC->GetAllAttributes(Attributes);

	for (const FGameplayAttribute& Attribute : Attributes)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
	}

	// GameplayEffect 관련 모든 델리게이트 해제
	ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll(this);
	ASC->OnAnyGameplayEffectRemovedDelegate().RemoveAll(this);

	// GameplayTag 변경 델리게이트 해제
	ASC->RegisterGenericGameplayTagEvent().RemoveAll(this);

	// 능력 커밋 델리게이트 해제
	ASC->AbilityCommittedCallbacks.RemoveAll(this);

	// GameplayEffectAddedHandles와 GameplayTagBoundToDelegates에 등록된 정보를 이용해 해제
	for (const FActiveGameplayEffectHandle GameplayEffectAddedHandle : GameplayEffectAddedHandles)
	{
		if (GameplayEffectAddedHandle.IsValid())
		{
			FOnActiveGameplayEffectStackChange* EffectStackChangeDelegate = ASC->OnGameplayEffectStackChangeDelegate(GameplayEffectAddedHandle);
			if (EffectStackChangeDelegate)
			{
				EffectStackChangeDelegate->RemoveAll(this);
			}

			FOnActiveGameplayEffectTimeChange* EffectTimeChangeDelegate = ASC->OnGameplayEffectTimeChangeDelegate(GameplayEffectAddedHandle);
			if (EffectTimeChangeDelegate)
			{
				EffectTimeChangeDelegate->RemoveAll(this);
			}
		}
	}

	for (const FGameplayTag GameplayTagBoundToDelegate : GameplayTagBoundToDelegates)
	{
		ASC->RegisterGameplayTagEvent(GameplayTagBoundToDelegate).RemoveAll(this);
	}
}

void UGSCCoreComponent::HandleDamage(const float DamageAmount, const FGameplayTagContainer& DamageTags, AActor* SourceActor)
{
	// 데미지 이벤트 발생 델리게이트
	OnDamage.Broadcast(DamageAmount, SourceActor, DamageTags);

	// TODO: 데미지 속성이 서버에서만 존재할 수 있어, 클라이언트에 동기화하는 방법을 추가로 고려해야 함
	// (원격 클라이언트에 BroadcastDamageToStatusBar 등의 추가 로직이 필요할 수 있음)
}

void UGSCCoreComponent::HandleHealthChange(const float DeltaValue, const FGameplayTagContainer& EventTags)
{
	// 초기 능력 부여 세팅이 끝나지 않았다면 이벤트를 호출하지 않음
	if (!bStartupAbilitiesGranted)
	{
		return;
	}

	// 체력 변경 이벤트 델리게이트
	OnHealthChange.Broadcast(DeltaValue, EventTags);

	// 체력이 0 이하이면 Die() 호출
	if (!IsAlive())
	{
		Die();
	}
}

void UGSCCoreComponent::HandleStaminaChange(const float DeltaValue, const FGameplayTagContainer& EventTags)
{
	// 초기 능력 부여 세팅이 끝나지 않았다면 이벤트를 호출하지 않음
	if (!bStartupAbilitiesGranted)
	{
		return;
	}

	// 스태미나 변경 이벤트 델리게이트
	OnStaminaChange.Broadcast(DeltaValue, EventTags);
}

void UGSCCoreComponent::HandleManaChange(const float DeltaValue, const FGameplayTagContainer& EventTags)
{
	if (!bStartupAbilitiesGranted)
	{
		return;
	}

	// 마나 변경 이벤트 델리게이트
	OnManaChange.Broadcast(DeltaValue, EventTags);
}

void UGSCCoreComponent::HandleAttributeChange(const FGameplayAttribute Attribute, const float DeltaValue, const FGameplayTagContainer& EventTags)
{
	// 모든 속성 변화 이벤트 델리게이트
	OnAttributeChange.Broadcast(Attribute, DeltaValue, EventTags);
}

void UGSCCoreComponent::OnAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 새 값과 이전 값
	const float NewValue = Data.NewValue;
	const float OldValue = Data.OldValue;

	// 값이 동일하다면(클램핑 등의 이유로 변화가 없었다면) 브로드캐스트하지 않음
	if (OldValue == NewValue)
	{
		return;
	}

	// 변화가 발생한 GameplayEffect 정보
	const FGameplayEffectModCallbackData* ModData = Data.GEModData;
	FGameplayTagContainer SourceTags;
	if (ModData)
	{
		// 소스 태그를 추출
		SourceTags = *ModData->EffectSpec.CapturedSourceTags.GetAggregatedTags();
	}

	// AttributeChange 이벤트 발생
	OnAttributeChange.Broadcast(Data.Attribute, NewValue - OldValue, SourceTags);
}

void UGSCCoreComponent::OnDamageAttributeChanged(const FOnAttributeChangeData& Data)
{
	// 필요 시 데미지 속성에 대한 추가 로직을 여기에 작성
	// 현재는 별도 처리 없이 리턴
}

void UGSCCoreComponent::Die()
{
	OnDeath.Broadcast();
}

float UGSCCoreComponent::GetHealth() const
{
	if (!OwnerAbilitySystemComponent)
	{
		return 0.0f;
	}

	return GetCurrentAttributeValue(UGSCAttributeSet::GetHealthAttribute());
}

float UGSCCoreComponent::GetMaxHealth() const
{
	if (!OwnerAbilitySystemComponent)
	{
		return 0.0f;
	}

	return GetCurrentAttributeValue(UGSCAttributeSet::GetMaxHealthAttribute());
}

float UGSCCoreComponent::GetStamina() const
{
	if (!OwnerAbilitySystemComponent)
	{
		return 0.0f;
	}

	return GetCurrentAttributeValue(UGSCAttributeSet::GetStaminaAttribute());
}

float UGSCCoreComponent::GetMaxStamina() const
{
	if (!OwnerAbilitySystemComponent)
	{
		return 0.0f;
	}

	return GetCurrentAttributeValue(UGSCAttributeSet::GetMaxStaminaAttribute());
}

float UGSCCoreComponent::GetMana() const
{
	if (!OwnerAbilitySystemComponent)
	{
		return 0.0f;
	}

	return GetCurrentAttributeValue(UGSCAttributeSet::GetManaAttribute());
}

float UGSCCoreComponent::GetMaxMana() const
{
	if (!OwnerAbilitySystemComponent)
	{
		return 0.0f;
	}

	return GetCurrentAttributeValue(UGSCAttributeSet::GetMaxManaAttribute());
}

float UGSCCoreComponent::GetAttributeValue(const FGameplayAttribute Attribute) const
{
	if (!OwnerAbilitySystemComponent)
	{
		GSC_WLOG(Error, TEXT("Owner AbilitySystemComponent가 유효하지 않습니다. 0.f 반환."));
		return 0.0f;
	}

	if (!Attribute.IsValid())
	{
		GSC_WLOG(Error, TEXT("유효하지 않은 Attribute(필드가 None). 0.f 반환."));
		return 0.f;
	}

	if (!OwnerAbilitySystemComponent->HasAttributeSetForAttribute(Attribute))
	{
		const UClass* AttributeSet = Attribute.GetAttributeSetClass();
		// 로그를 한글로 변환
		GSC_WLOG(
			Error,
			TEXT("속성 [%s.%s] 값을 조회하려 하지만, %s가 %s에 부여되지 않았습니다. 0.f 반환."),
			*GetNameSafe(AttributeSet),
			*Attribute.GetName(),
			*GetNameSafe(AttributeSet),
			*GetNameSafe(OwnerAbilitySystemComponent)
		);

		return 0.f;
	}

	return OwnerAbilitySystemComponent->GetNumericAttributeBase(Attribute);
}

float UGSCCoreComponent::GetCurrentAttributeValue(const FGameplayAttribute Attribute) const
{
	if (!OwnerAbilitySystemComponent)
	{
		GSC_WLOG(Error, TEXT("Owner AbilitySystemComponent가 유효하지 않습니다. 0.f 반환."));
		return 0.0f;
	}

	if (!Attribute.IsValid())
	{
		GSC_WLOG(Error, TEXT("유효하지 않은 Attribute(필드가 None). 0.f 반환."));
		return 0.f;
	}

	if (!OwnerAbilitySystemComponent->HasAttributeSetForAttribute(Attribute))
	{
		const UClass* AttributeSet = Attribute.GetAttributeSetClass();
		GSC_WLOG(
			Error,
			TEXT("속성 [%s.%s] 값을 조회하려 하지만, %s가 %s에 부여되지 않았습니다. 0.f 반환."),
			*GetNameSafe(AttributeSet),
			*Attribute.GetName(),
			*GetNameSafe(AttributeSet),
			*GetNameSafe(OwnerAbilitySystemComponent)
		);
		return 0.0f;
	}

	return OwnerAbilitySystemComponent->GetNumericAttribute(Attribute);
}

bool UGSCCoreComponent::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void UGSCCoreComponent::GrantAbility(TSubclassOf<UGameplayAbility> Ability, int32 Level)
{
	// 소유 액터, ASC, 그리고 Ability가 유효해야 함
	if (!OwnerActor || !OwnerAbilitySystemComponent || !Ability)
	{
		return;
	}

	// 서버 권한인지 확인
	if (!OwnerAbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		GSC_LOG(Warning, TEXT("UGSCCoreComponent::GrantAbility가 권한이 없는 곳(Non-authority)에서 호출됨"));
		return;
	}

	// AbilitySpec 생성
	FGameplayAbilitySpec Spec;
	Spec.Ability = Ability.GetDefaultObject();

	// 레벨/주인 Actor를 설정하여 실제 AbilitySpec을 부여
	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability, Level, INDEX_NONE, OwnerActor);
	OwnerAbilitySystemComponent->GiveAbility(AbilitySpec);
}

void UGSCCoreComponent::ClearAbility(const TSubclassOf<UGameplayAbility> Ability)
{
	// 내부적으로 배열에 담아 ClearAbilities를 호출
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesToRemove;
	AbilitiesToRemove.Add(Ability);
	return ClearAbilities(AbilitiesToRemove);
}

void UGSCCoreComponent::ClearAbilities(const TArray<TSubclassOf<UGameplayAbility>> Abilities)
{
	// 소유 Actor와 ASC가 유효하고, 서버 권한이어야 함
	if (!OwnerActor || !OwnerAbilitySystemComponent || !OwnerAbilitySystemComponent->IsOwnerActorAuthoritative())
	{
		return;
	}

	// 현재 ASC가 가진 모든 AbilitySpec을 탐색
	TArray<FGameplayAbilitySpecHandle> AbilitiesToRemove;
	for (const FGameplayAbilitySpec& Spec : OwnerAbilitySystemComponent->GetActivatableAbilities())
	{
		// 제거 리스트에 포함되어 있다면 Spec.Handle 추가
		if (Abilities.Contains(Spec.Ability->GetClass()))
		{
			AbilitiesToRemove.Add(Spec.Handle);
		}
	}

	// 실제 제거는 한 번에 처리
	for (const FGameplayAbilitySpecHandle AbilityToRemove : AbilitiesToRemove)
	{
		OwnerAbilitySystemComponent->ClearAbility(AbilityToRemove);
	}
}

bool UGSCCoreComponent::HasAnyMatchingGameplayTags(const FGameplayTagContainer TagContainer) const
{
	if (OwnerAbilitySystemComponent)
	{
		return OwnerAbilitySystemComponent->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool UGSCCoreComponent::HasMatchingGameplayTag(const FGameplayTag TagToCheck) const
{
	if (OwnerAbilitySystemComponent)
	{
		return OwnerAbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool UGSCCoreComponent::IsUsingAbilityByClass(const TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass)
	{
		GSC_LOG(Error, TEXT("UGSCCoreComponent::IsUsingAbilityByClass()에 null AbilityClass가 전달됨"));
		return false;
	}

	return GetActiveAbilitiesByClass(AbilityClass).Num() > 0;
}

bool UGSCCoreComponent::IsUsingAbilityByTags(const FGameplayTagContainer AbilityTags)
{
	if (!OwnerAbilitySystemComponent)
	{
		GSC_LOG(Error, TEXT("UGSCCoreComponent::IsUsingAbilityByTags() ASC가 유효하지 않음"));
		return false;
	}

	return GetActiveAbilitiesByTags(AbilityTags).Num() > 0;
}

TArray<UGameplayAbility*> UGSCCoreComponent::GetActiveAbilitiesByClass(TSubclassOf<UGameplayAbility> AbilityToSearch) const
{
	if (!OwnerAbilitySystemComponent)
	{
		GSC_LOG(Error, TEXT("UGSCCoreComponent::GetActiveAbilitiesByClass() ASC가 유효하지 않음"));
		return {};
	}

	// ASC가 보유한 모든 AbilitySpec을 가져옴
	TArray<FGameplayAbilitySpec> Specs = OwnerAbilitySystemComponent->GetActivatableAbilities();
	TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;
	TArray<UGameplayAbility*> ActiveAbilities;

	// 지정한 클래스(AbilityToSearch)와 매칭되는 스펙만 추려냄
	for (const FGameplayAbilitySpec& Spec : Specs)
	{
		if (Spec.Ability && Spec.Ability->GetClass()->IsChildOf(AbilityToSearch))
		{
			MatchingGameplayAbilities.Add(const_cast<FGameplayAbilitySpec*>(&Spec));
		}
	}

	// 매칭된 스펙에서 활성 인스턴스를 찾음
	for (const FGameplayAbilitySpec* Spec : MatchingGameplayAbilities)
	{
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			// 활성 상태인지 확인
			if (ActiveAbility->IsActive())
			{
				ActiveAbilities.Add(ActiveAbility);
			}
		}
	}

	return ActiveAbilities;
}

TArray<UGameplayAbility*> UGSCCoreComponent::GetActiveAbilitiesByTags(const FGameplayTagContainer GameplayTagContainer) const
{
	if (!OwnerAbilitySystemComponent)
	{
		GSC_LOG(Error, TEXT("UGSCCoreComponent::GetActiveAbilitiesByTags() ASC가 유효하지 않음"));
		return {};
	}

	TArray<UGameplayAbility*> ActiveAbilities;
	TArray<FGameplayAbilitySpec*> MatchingGameplayAbilities;

	// ASC에서 태그를 만족하는 AbilitySpec들을 모두 찾음
	OwnerAbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(GameplayTagContainer, MatchingGameplayAbilities, false);

	for (const FGameplayAbilitySpec* Spec : MatchingGameplayAbilities)
	{
		TArray<UGameplayAbility*> AbilityInstances = Spec->GetAbilityInstances();

		for (UGameplayAbility* ActiveAbility : AbilityInstances)
		{
			if (ActiveAbility->IsActive())
			{
				ActiveAbilities.Add(ActiveAbility);
			}
		}
	}

	return ActiveAbilities;
}

bool UGSCCoreComponent::ActivateAbilityByClass(const TSubclassOf<UGameplayAbility> AbilityClass, UGSCGameplayAbility*& ActivatedAbility, const bool bAllowRemoteActivation)
{
	if (!OwnerAbilitySystemComponent || !AbilityClass)
	{
		return false;
	}

	// Ability 활성화 시도
	const bool bSuccess = OwnerAbilitySystemComponent->TryActivateAbilityByClass(AbilityClass, bAllowRemoteActivation);

	// 활성화된 Ability 인스턴스 목록 조회
	TArray<UGameplayAbility*> ActiveAbilities = GetActiveAbilitiesByClass(AbilityClass);
	if (ActiveAbilities.Num() == 0)
	{
		GSC_LOG(Verbose, TEXT("UGSCCoreComponent::ActivateAbilityByClass 클래스 %s에 대응되는 활성 Ability 없음"), *AbilityClass->GetName());
	}

	// 성공했고, 활성 Ability가 있다면 첫 번째 인스턴스를 UGSCGameplayAbility로 캐스팅
	if (bSuccess && ActiveAbilities.Num() > 0)
	{
		if (UGSCGameplayAbility* GSCAbility = Cast<UGSCGameplayAbility>(ActiveAbilities[0]))
		{
			ActivatedAbility = GSCAbility;
		}
	}

	return bSuccess;
}

bool UGSCCoreComponent::ActivateAbilityByTags(const FGameplayTagContainer AbilityTags, UGSCGameplayAbility*& ActivatedAbility, const bool bAllowRemoteActivation)
{
	if (!OwnerAbilitySystemComponent)
	{
		return false;
	}

	// 태그에 매칭되는 AbilitySpec 목록을 가져옴
	TArray<FGameplayAbilitySpec*> AbilitiesToActivate;
	OwnerAbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTags, AbilitiesToActivate);

	const uint32 Count = AbilitiesToActivate.Num();
	if (Count == 0)
	{
		GSC_LOG(Warning, TEXT("UGSCCoreComponent::ActivateAbilityByTags 태그 %s를 만족하는 Ability 없음"), *AbilityTags.ToStringSimple())
		return false;
	}

	// 여러 Ability 중 임의로 하나를 선택하여 활성화 시도
	const FGameplayAbilitySpec* Spec = AbilitiesToActivate[FMath::RandRange(0, Count - 1)];
	const bool bSuccess = OwnerAbilitySystemComponent->TryActivateAbility(Spec->Handle, bAllowRemoteActivation);

	// 활성화 후 실제 인스턴스를 가져와서 반환
	TArray<UGameplayAbility*> ActiveAbilities = GetActiveAbilitiesByTags(AbilityTags);
	if (ActiveAbilities.Num() == 0)
	{
		GSC_LOG(Warning, TEXT("UGSCCoreComponent::ActivateAbilityByTags 태그 %s를 가진 활성 Ability를 찾지 못함"), *AbilityTags.ToStringSimple());
	}

	if (bSuccess && ActiveAbilities.Num() > 0)
	{
		if (UGSCGameplayAbility* GSCAbility = Cast<UGSCGameplayAbility>(ActiveAbilities[0]))
		{
			ActivatedAbility = GSCAbility;
		}
	}

	return bSuccess;
}

void UGSCCoreComponent::SetAttributeValue(const FGameplayAttribute Attribute, const float NewValue)
{
	if (!OwnerAbilitySystemComponent)
	{
		GSC_LOG(Warning, TEXT("SetAttributeValue() Owner AbilitySystemComponent가 유효하지 않아 중단합니다."));
		return;
	}

	OwnerAbilitySystemComponent->SetNumericAttributeBase(Attribute, NewValue);
}

void UGSCCoreComponent::ClampAttributeValue(const FGameplayAttribute Attribute, const float MinValue, const float MaxValue)
{
	// 현재 속성 값을 가져온 뒤 범위로 제한
	const float CurrentValue = GetAttributeValue(Attribute);
	const float NewValue = FMath::Clamp(CurrentValue, MinValue, MaxValue);
	SetAttributeValue(Attribute, NewValue);
}

void UGSCCoreComponent::AdjustAttributeForMaxChange(UGSCAttributeSetBase* AttributeSet, const FGameplayAttribute AffectedAttributeProperty, const FGameplayAttribute MaxAttribute, const float NewMaxValue)
{
	if (!OwnerAbilitySystemComponent)
	{
		GSC_LOG(Warning, TEXT("AdjustAttributeForMaxChange() Owner AbilitySystemComponent가 유효하지 않아 중단합니다."));
		return;
	}

	// 변경 대상 속성과 최대 속성 데이터를 가져옴
	FGameplayAttributeData* AttributeData = AffectedAttributeProperty.GetGameplayAttributeData(AttributeSet);
	if (!AttributeData)
	{
		GSC_LOG(Warning, TEXT("AdjustAttributeForMaxChange() AffectedAttributeProperty에서 AttributeData를 가져올 수 없습니다."));
		return;
	}

	const FGameplayAttributeData* MaxAttributeData = MaxAttribute.GetGameplayAttributeData(AttributeSet);
	if (!AttributeData)
	{
		GSC_LOG(Warning, TEXT("AdjustAttributeForMaxChange() MaxAttribute에서 AttributeData를 가져올 수 없습니다."));
		return;
	}

	// AttributeSet 내부 로직을 통해 비례 조정
	AttributeSet->AdjustAttributeForMaxChange(*AttributeData, *MaxAttributeData, NewMaxValue, AffectedAttributeProperty);
}

void UGSCCoreComponent::PreAttributeChange(UGSCAttributeSetBase* AttributeSet, const FGameplayAttribute& Attribute, const float NewValue)
{
	OnPreAttributeChange.Broadcast(AttributeSet, Attribute, NewValue);
}

void UGSCCoreComponent::PostGameplayEffectExecute(UGSCAttributeSetBase* AttributeSet, const FGameplayEffectModCallbackData& Data)
{
	if (!AttributeSet)
	{
		GSC_LOG(Error, TEXT("UGSCCoreComponent::PostGameplayEffectExecute()에서 AttributeSet이 유효하지 않음"));
		return;
	}

	// 소스 액터와 타겟 액터(해당 AttributeSet 보유)를 가져옴
	AActor* SourceActor = nullptr;
	AActor* TargetActor = nullptr;
	AttributeSet->GetSourceAndTargetFromContext<AActor>(Data, SourceActor, TargetActor);

	// 소스 태그
	const FGameplayTagContainer SourceTags = AttributeSet->GetSourceTagsFromContext(Data);
	const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();

	// 속성의 최소 클램프 값(있다면)
	const float ClampMinimumValue = AttributeSet->GetClampMinimumValueFor(Data.EvaluatedData.Attribute);

	// New - Old에 해당하는 DeltaValue 계산(보통 Additive일 경우)
	float DeltaValue = 0;
	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
	{
		DeltaValue = Data.EvaluatedData.Magnitude;
	}

	// 델리게이트로 BP에 전달할 Payload 구성
	FGSCGameplayEffectExecuteData Payload;
	Payload.AttributeSet = AttributeSet;
	Payload.AbilitySystemComponent = AttributeSet->GetOwningAbilitySystemComponent();
	Payload.DeltaValue = DeltaValue;
	Payload.ClampMinimumValue = ClampMinimumValue;

	// OnPostGameplayEffectExecute 이벤트 방송
	OnPostGameplayEffectExecute.Broadcast(Data.EvaluatedData.Attribute, SourceActor, TargetActor, SourceTags, Payload);
}

void UGSCCoreComponent::SetStartupAbilitiesGranted(const bool bGranted)
{
	bStartupAbilitiesGranted = bGranted;
}

void UGSCCoreComponent::OnActiveGameplayEffectAdded(UAbilitySystemComponent* Target, const FGameplayEffectSpec& SpecApplied, const FActiveGameplayEffectHandle ActiveHandle)
{
	if (!OwnerAbilitySystemComponent)
		return;

	// AssetTags, GrantedTags 추출
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	// GameplayEffect가 추가되었다고 브로드캐스트
	OnGameplayEffectAdded.Broadcast(AssetTags, GrantedTags, ActiveHandle);

	// 스택 변화, 시간 변화 델리게이트 바인딩
	if (FOnActiveGameplayEffectStackChange* Delegate = OwnerAbilitySystemComponent->OnGameplayEffectStackChangeDelegate(ActiveHandle))
	{
		Delegate->AddUObject(this, &UGSCCoreComponent::OnActiveGameplayEffectStackChanged);
	}
	if (FOnActiveGameplayEffectTimeChange* Delegate = OwnerAbilitySystemComponent->OnGameplayEffectTimeChangeDelegate(ActiveHandle))
	{
		Delegate->AddUObject(this, &UGSCCoreComponent::OnActiveGameplayEffectTimeChanged);
	}

	// 해제 시 참조하기 위해 저장
	GameplayEffectAddedHandles.AddUnique(ActiveHandle);
}

void UGSCCoreComponent::OnActiveGameplayEffectStackChanged(const FActiveGameplayEffectHandle ActiveHandle, const int32 NewStackCount, const int32 PreviousStackCount)
{
	if (!OwnerAbilitySystemComponent)
		return;

	// ASC에서 해당 핸들의 GameplayEffect를 찾음
	const FActiveGameplayEffect* GameplayEffect = OwnerAbilitySystemComponent->GetActiveGameplayEffect(ActiveHandle);
	if (!GameplayEffect)
		return;

	FGameplayTagContainer AssetTags;
	GameplayEffect->Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	GameplayEffect->Spec.GetAllGrantedTags(GrantedTags);

	// 스택 개수 변경 이벤트 브로드캐스트
	OnGameplayEffectStackChange.Broadcast(AssetTags, GrantedTags, ActiveHandle, NewStackCount, PreviousStackCount);
}

void UGSCCoreComponent::OnActiveGameplayEffectTimeChanged(const FActiveGameplayEffectHandle ActiveHandle, const float NewStartTime, const float NewDuration)
{
	if (!OwnerAbilitySystemComponent)
		return;

	// ASC에서 해당 핸들의 GameplayEffect를 찾음
	const FActiveGameplayEffect* GameplayEffect = OwnerAbilitySystemComponent->GetActiveGameplayEffect(ActiveHandle);
	if (!GameplayEffect)
		return;

	FGameplayTagContainer AssetTags;
	GameplayEffect->Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	GameplayEffect->Spec.GetAllGrantedTags(GrantedTags);

	// 시간 변화 이벤트 브로드캐스트
	OnGameplayEffectTimeChange.Broadcast(AssetTags, GrantedTags, ActiveHandle, NewStartTime, NewDuration);
}

void UGSCCoreComponent::OnAnyGameplayEffectRemoved(const FActiveGameplayEffect& EffectRemoved)
{
	if (!OwnerAbilitySystemComponent)
		return;

	FGameplayTagContainer AssetTags;
	EffectRemoved.Spec.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	EffectRemoved.Spec.GetAllGrantedTags(GrantedTags);

	// 스택 0, 이전 스택 1로 간주하여 이벤트 호출
	OnGameplayEffectStackChange.Broadcast(AssetTags, GrantedTags, EffectRemoved.Handle, 0, 1);

	// 제거 이벤트 브로드캐스트
	OnGameplayEffectRemoved.Broadcast(AssetTags, GrantedTags, EffectRemoved.Handle);
}

void UGSCCoreComponent::OnAnyGameplayTagChanged(const FGameplayTag GameplayTag, const int32 NewCount) const
{
	// 태그 변경 이벤트 브로드캐스트
	OnGameplayTagChange.Broadcast(GameplayTag, NewCount);
}

void UGSCCoreComponent::OnAbilityCommitted(UGameplayAbility* ActivatedAbility)
{
	if (!ActivatedAbility)
	{
		return;
	}

	// 능력 커밋 이벤트 브로드캐스트
	OnAbilityCommit.Broadcast(ActivatedAbility);

	// 쿨다운 처리 로직
	HandleCooldownOnAbilityCommit(ActivatedAbility);
}

void UGSCCoreComponent::OnCooldownGameplayTagChanged(const FGameplayTag GameplayTag, const int32 NewCount, const FGameplayAbilitySpecHandle AbilitySpecHandle, const float Duration)
{
	// 쿨다운 태그 카운트가 0이 아니라면(아직 쿨다운 중이라면) 반환
	if (NewCount != 0)
		return;

	if (!OwnerAbilitySystemComponent)
		return;
	
	// 해당 AbilitySpec을 ASC에서 찾음
	FGameplayAbilitySpec* AbilitySpec = OwnerAbilitySystemComponent->FindAbilitySpecFromHandle(AbilitySpecHandle);
	if (!AbilitySpec)
	{
		// 쿨다운 만료 시 Ability가 제거되었을 수 있음
		return;
	}

	UGameplayAbility* Ability = AbilitySpec->Ability;

	// 쿨다운 종료 이벤트 브로드캐스트
	if (IsValid(Ability))
	{
		OnCooldownEnd.Broadcast(Ability, GameplayTag, Duration);
	}

	// 쿨다운 태그 관련 델리게이트 제거
	OwnerAbilitySystemComponent->RegisterGameplayTagEvent(GameplayTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
}

void UGSCCoreComponent::HandleCooldownOnAbilityCommit(UGameplayAbility* ActivatedAbility)
{
	if (!OwnerAbilitySystemComponent)
	{
		return;
	}

	if (!IsValid(ActivatedAbility))
	{
		GSC_LOG(Warning, TEXT("UGSCCoreComponent::HandleCooldownOnAbilityCommit()에서 ActivatedAbility가 유효하지 않음"))
		return;
	}

	// Ability가 사용하는 쿨다운 GameplayEffect 가져오기
	UGameplayEffect* CooldownGE = ActivatedAbility->GetCooldownGameplayEffect();
	if (!CooldownGE)
	{
		return;
	}

	// 인스턴스화된 Ability인지 확인
	if (!ActivatedAbility->IsInstantiated())
	{
		return;
	}

	// 쿨다운 태그 컨테이너 가져오기
	const FGameplayTagContainer* CooldownTags = ActivatedAbility->GetCooldownTags();
	if (!CooldownTags || CooldownTags->Num() <= 0)
	{
		return;
	}

	// ASC ActorInfo와 AbilitySpecHandle
	FGameplayAbilityActorInfo ActorInfo = ActivatedAbility->GetActorInfo();
	const FGameplayAbilitySpecHandle AbilitySpecHandle = ActivatedAbility->GetCurrentAbilitySpecHandle();

	// 남은 시간(TimeRemaining)과 전체 지속(Duration) 구하기
	float TimeRemaining = 0.f;
	float Duration = 0.f;
	ActivatedAbility->GetCooldownTimeRemainingAndDuration(AbilitySpecHandle, &ActorInfo, TimeRemaining, Duration);

	// 쿨다운 시작 이벤트 브로드캐스트
	OnCooldownStart.Broadcast(ActivatedAbility, *CooldownTags, TimeRemaining, Duration);

	// 쿨다운 태그가 소진될(카운트가 0이 될) 때를 감지하기 위한 델리게이트 등록
	TArray<FGameplayTag> GameplayTags;
	CooldownTags->GetGameplayTagArray(GameplayTags);
	for (const FGameplayTag GameplayTag : GameplayTags)
	{
		OwnerAbilitySystemComponent->RegisterGameplayTagEvent(GameplayTag)
		                           .AddUObject(this, &UGSCCoreComponent::OnCooldownGameplayTagChanged, AbilitySpecHandle, Duration);

		// 해제 시점을 위한 내부 배열에 저장
		GameplayTagBoundToDelegates.AddUnique(GameplayTag);
	}
}
