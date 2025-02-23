#include "Abilities/GSCGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GSCTargetType.h"
#include "Components/GSCAbilityQueueComponent.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "GSCLog.h"


UGSCGameplayAbility::UGSCGameplayAbility()
{
}

FGSCGameplayEffectContainerSpec UGSCGameplayAbility::MakeEffectContainerSpecFromContainer(const FGSCGameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	// 결과로 반환할 효과 컨테이너 사양 구조체를 선언 및 초기화
	FGSCGameplayEffectContainerSpec ReturnSpec;

	// 소유자 액터와 해당 액터의 AbilitySystemComponent를 검색합니다.
	const AActor* OwningActor = GetOwningActorFromActorInfo();

	// AbilitySystemComponent가 유효한 경우에만 효과 사양을 생성합니다.
	if (UAbilitySystemComponent* OwningASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor))
	{
		// 대상(Target) 설정: Container에 TargetType이 설정되어 있으면 타겟팅 로직 실행
		if (Container.TargetType.Get())
		{
			// 히트 결과와 대상 액터 배열을 선언합니다.
			TArray<FHitResult> HitResults;
			TArray<AActor*> TargetActors;

			// TargetType의 기본 객체(CDO)를 가져와서 타겟팅 로직을 수행합니다.
			const UGSCTargetType* TargetTypeCDO = Container.TargetType.GetDefaultObject();

			// 아바타 액터(능력 실행 주체)를 가져옵니다.
			AActor* AvatarActor = GetAvatarActorFromActorInfo();

			// 타겟팅 로직을 통해 HitResults 및 TargetActors 배열을 채웁니다.
			TargetTypeCDO->GetTargets(AvatarActor, EventData, HitResults, TargetActors);

			// 얻은 대상들을 효과 컨테이너 사양에 추가합니다.
			ReturnSpec.AddTargets(HitResults, TargetActors);
		}

		// OverrideGameplayLevel이 INDEX_NONE인 경우, 현재 능력의 레벨을 사용합니다.
		if (OverrideGameplayLevel == INDEX_NONE)
		{
			OverrideGameplayLevel = GetAbilityLevel();
		}

		// 컨테이너에 정의된 각 GameplayEffect 클래스를 대상으로 효과 사양을 생성합니다.
		for (const TSubclassOf<UGameplayEffect>& EffectClass : Container.TargetGameplayEffectClasses)
		{
			// 지정된 효과 클래스와 레벨을 사용하여 효과 사양 핸들을 생성합니다.
			FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(EffectClass, OverrideGameplayLevel);

			// 생성된 효과 사양을 가져옵니다.
			FGameplayEffectSpec* Spec = SpecHandle.Data.Get();
			// 만약 효과 사양이 유효하고, 컨테이너에서 SetByCaller Magnitude 사용이 지정되어 있다면, 해당 값을 설정합니다.
			if (Spec && Container.bUseSetByCallerMagnitude)
			{
				Spec->SetSetByCallerMagnitude(Container.SetByCallerDataTag, Container.SetByCallerMagnitude);
			}
			// 생성된 효과 사양 핸들을 결과 사양 구조체의 배열에 추가합니다.
			ReturnSpec.TargetGameplayEffectSpecs.Add(SpecHandle);
		}
	}
	// 최종적으로 생성된 효과 컨테이너 사양을 반환합니다.
	return ReturnSpec;
}

FGSCGameplayEffectContainerSpec UGSCGameplayAbility::MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	// EffectContainerMap에서 ContainerTag에 해당하는 효과 컨테이너를 검색합니다.
	FGSCGameplayEffectContainer* FoundContainer = EffectContainerMap.Find(ContainerTag);

	// 만약 효과 컨테이너를 찾았다면, 해당 컨테이너를 이용하여 효과 사양을 생성합니다.
	if (FoundContainer)
	{
		return MakeEffectContainerSpecFromContainer(*FoundContainer, EventData, OverrideGameplayLevel);
	}
	// 찾지 못하면, 빈 효과 컨테이너 사양을 반환합니다.
	return FGSCGameplayEffectContainerSpec();
}

TArray<FActiveGameplayEffectHandle> UGSCGameplayAbility::ApplyEffectContainerSpec(const FGSCGameplayEffectContainerSpec& ContainerSpec)
{
	// 모든 효과 핸들을 저장할 배열을 선언합니다.
	TArray<FActiveGameplayEffectHandle> AllEffects;

	// ContainerSpec에 포함된 각 효과 사양 핸들을 순회하면서,
	// 대상 데이터(ContainerSpec.TargetData)에 효과를 적용하고, 결과 핸들을 배열에 추가합니다.
	for (const FGameplayEffectSpecHandle& SpecHandle : ContainerSpec.TargetGameplayEffectSpecs)
	{
		// K2_ApplyGameplayEffectSpecToTarget 함수는 효과를 대상에 적용하고, 적용된 효과 핸들의 배열을 반환합니다.
		AllEffects.Append(K2_ApplyGameplayEffectSpecToTarget(SpecHandle, ContainerSpec.TargetData));
	}
	// 적용된 모든 효과 핸들을 반환합니다.
	return AllEffects;
}

TArray<FActiveGameplayEffectHandle> UGSCGameplayAbility::ApplyEffectContainer(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel)
{
	// 먼저 효과 컨테이너 사양을 생성합니다.
	const FGSCGameplayEffectContainerSpec Spec = MakeEffectContainerSpec(ContainerTag, EventData, OverrideGameplayLevel);

	// 생성된 사양을 적용하여 효과를 활성화한 후, 결과 효과 핸들을 반환합니다.
	return ApplyEffectContainerSpec(Spec);
}

void UGSCGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	// 기본 UGameplayAbility의 OnAvatarSet을 호출하여 기본 처리를 수행합니다.
	Super::OnAvatarSet(ActorInfo, Spec);

	// 만약 부여 시 자동 활성화가 활성화되어 있다면, 능력을 즉시 활성화합니다.
	if (bActivateOnGranted)
	{
		ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}

bool UGSCGameplayAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	// 비용 검사 완화 플래그가 true이면, 비용 검사 없이 true를 반환합니다.
	if (bLooselyCheckAbilityCost)
	{
		return true;
	}

	// 그렇지 않으면, 기본 CheckCost 구현을 사용합니다.
	return Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags);
}

bool UGSCGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
                                             FGameplayTagContainer* OptionalRelevantTags) const
{
	// 기본 활성화 검사 결과를 가져옵니다.
	const bool bCanActivateAbility = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	if (!bCanActivateAbility)
	{
		return false;
	}

	// 만약 Blueprint에서 별도의 사용 검사를 구현하지 않았고, 비용 검사 완화가 활성화되어 있다면,
	// 비용이 양수인지 검사하여 능력 활성 여부를 결정합니다.
	if (!bHasBlueprintCanUse && bLooselyCheckAbilityCost)
	{
		return CheckForPositiveCost(Handle, ActorInfo, OptionalRelevantTags);
	}

	// 위의 조건을 모두 만족하면 능력 활성화 가능
	return true;
}

void UGSCGameplayAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
                                      const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate);

	OnGameplayAbilityEnded.AddUObject(this, &UGSCGameplayAbility::AbilityEnded);

	// 능력 큐 사용이 활성화되어 있지 않으면 사전 처리를 종료합니다.
	if (!bEnableAbilityQueue)
		return;

	// 아바타 액터를 가져옵니다.
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
		return;

	// 아바타에서 AbilityQueueComponent를 검색합니다.
	UGSCAbilityQueueComponent* AbilityQueueComponent = UGSCBlueprintFunctionLibrary::GetAbilityQueueComponent(Avatar);
	if (!AbilityQueueComponent)
	{
		return;
	}

	GSC_LOG(Log, TEXT("UGSCGameplayAbility::PreActivate %s, 능력 큐 열기"), *GetName())

	// 능력 큐를 열고, 모든 능력 사용을 허용합니다.
	AbilityQueueComponent->OpenAbilityQueue();
	AbilityQueueComponent->SetAllowAllAbilitiesForAbilityQueue(true);
}

void UGSCGameplayAbility::AbilityEnded(UGameplayAbility* Ability)
{
	GSC_LOG(Log, TEXT("UGSCGameplayAbility::AbilityEnded"))

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		GSC_LOG(Warning, TEXT("UGSCGameplayAbility::AbilityEnded, 아바타 액터를 찾을 수 없습니다."));
		return;
	}

	GSC_LOG(Log, TEXT("UGSCGameplayAbility::AbilityEnded, 델리게이트 브로드캐스트"));
	OnAbilityEnded.Broadcast();
	OnAbilityEnded.Clear();
}

bool UGSCGameplayAbility::CheckForPositiveCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	// 비용 GameplayEffect를 가져옵니다.
	UGameplayEffect* CostGE = GetCostGameplayEffect();

	// 비용 효과가 없으면 비용 검사를 통과합니다.
	if (!CostGE)
	{
		return true;
	}

	// 효과를 적용할 수 있는지 검사합니다.
	if (!CanApplyPositiveAttributeModifiers(CostGE, ActorInfo, GetAbilityLevel(Handle, ActorInfo), MakeEffectContext(Handle, ActorInfo)))
	{
		// 비용 실패 태그를 가져옵니다.
		const FGameplayTag& CostTag = UAbilitySystemGlobals::Get().ActivateFailCostTag;

		// OptionalRelevantTags가 유효하고 비용 실패 태그가 유효하면 추가합니다.
		if (OptionalRelevantTags && CostTag.IsValid())
		{
			OptionalRelevantTags->AddTag(CostTag);
		}

		return false; // 비용 검사 실패
	}

	return true; // 모든 검사 통과
}

bool UGSCGameplayAbility::CanApplyPositiveAttributeModifiers(const UGameplayEffect* GameplayEffect, const FGameplayAbilityActorInfo* ActorInfo, const float Level, const FGameplayEffectContextHandle& EffectContext) const
{
	// GameplayEffectSpec을 생성합니다.
	FGameplayEffectSpec Spec(GameplayEffect, EffectContext, Level);

	// 수정자 magnitudes를 계산합니다.
	Spec.CalculateModifierMagnitudes();

	// AbilitySystemComponent를 가져옵니다.
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.IsValid() ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	if (!ASC)
	{
		// AbilitySystemComponent가 없으면 false 반환
		return false;
	}

	// 모든 수정자들을 순회합니다.
	for (int32 ModIdx = 0; ModIdx < Spec.Modifiers.Num(); ++ModIdx)
	{
		// 해당 수정자의 정의를 가져옵니다.
		const FGameplayModifierInfo& ModDef = Spec.Def->Modifiers[ModIdx];

		// Additive 연산자에 대해서만 검사합니다.
		if (ModDef.ModifierOp == EGameplayModOp::Additive)
		{
			// 수정자에 속성이 유효하지 않으면 검사 건너뜁니다.
			if (!ModDef.Attribute.IsValid())
			{
				continue;
			}

			// AbilitySystemComponent가 해당 속성을 보유하고 있는지 확인합니다.
			if (!ASC->HasAttributeSetForAttribute(ModDef.Attribute))
			{
				continue;
			}

			// ASC에서 해당 속성의 UAttributeSet을 검색합니다.
			const UAttributeSet* Set = GetAttributeSubobjectForASC(ASC, ModDef.Attribute.GetAttributeSetClass());
			// 현재 속성 값을 가져옵니다.
			const float CurrentValue = ModDef.Attribute.GetNumericValueChecked(Set);

			// 만약 속성 값이 0 이하이면, 수정자 적용 불가로 판단합니다.
			if (CurrentValue <= 0.f)
			{
				return false;
			}
		}
	}

	// 모든 Additive 수정자가 양수이면 true 반환
	return true;
}

const UAttributeSet* UGSCGameplayAbility::GetAttributeSubobjectForASC(UAbilitySystemComponent* AbilitySystemComponent, TSubclassOf<UAttributeSet> AttributeClass)
{
	// AbilitySystemComponent가 nullptr가 아니어야 합니다.
	check(AbilitySystemComponent != nullptr);

	// ASC에서 스폰된 모든 AttributeSet을 순회합니다.
	for (const UAttributeSet* Set : AbilitySystemComponent->GetSpawnedAttributes())
	{
		// Set이 유효하고, AttributeClass의 인스턴스라면 반환합니다.
		if (Set && Set->IsA(AttributeClass))
		{
			return Set;
		}
	}

	// 해당 AttributeSet을 찾지 못하면 nullptr 반환.
	return nullptr;
}
