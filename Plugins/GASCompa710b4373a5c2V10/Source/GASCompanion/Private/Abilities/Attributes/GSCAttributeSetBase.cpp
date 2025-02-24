#include "Abilities/Attributes/GSCAttributeSetBase.h"

#include "GSCLog.h"
#include "GameplayEffectExtension.h"
#include "Abilities/GSCBlueprintFunctionLibrary.h"
#include "Components/GSCCoreComponent.h"
#include "GameFramework/Pawn.h"


UGSCAttributeSetBase::UGSCAttributeSetBase()
{
	// 현재 특별한 초기화 로직은 없음.
	// 추후 기본값 설정이나 초기화 코드를 이곳에 추가할 수 있습니다.
}

void UGSCAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// 부모 클래스의 PreAttributeChange 호출: 기본 처리 수행
	Super::PreAttributeChange(Attribute, NewValue);

	// 현재 AttributeSet이 적용된 액터의 AbilityActorInfo를 가져옵니다.
	const FGameplayAbilityActorInfo* ActorInfo = GetActorInfo();
	if (!ActorInfo)
	{
		// ActorInfo가 유효하지 않으면 더 이상 처리하지 않습니다.
		return;
	}

	// ActorInfo에서 AvatarActor(실제 게임 플레이에서의 액터)를 가져옵니다.
	const TWeakObjectPtr<AActor> AvatarActorPtr = ActorInfo->AvatarActor;
	if (!AvatarActorPtr.IsValid())
	{
		// AvatarActor가 유효하지 않으면 처리를 중단합니다.
		return;
	}

	// 실제 AvatarActor 포인터를 가져옵니다.
	const AActor* AvatarActor = AvatarActorPtr.Get();
	// GAS Companion Core 컴포넌트를 통해 추가적인 속성 변경 처리를 요청합니다.
	if (UGSCCoreComponent* CoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(AvatarActor))
	{
		// CoreComponent의 PreAttributeChange 함수에 현재 AttributeSet, 속성, 새 값(NewValue)을 전달합니다.
		CoreComponent->PreAttributeChange(this, Attribute, NewValue);
	}
}

void UGSCAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	// 부모 클래스의 기본 후처리 호출
	Super::PostGameplayEffectExecute(Data);

	// 소스와 타겟 액터 포인터 초기화
	AActor* SourceActor = nullptr;
	AActor* TargetActor = nullptr;
	// 템플릿 함수를 사용하여, Data에서 AActor 타입의 소스 및 타겟 액터를 추출합니다.
	GetSourceAndTargetFromContext<AActor>(Data, SourceActor, TargetActor);

	// 타겟 액터에 부착된 GAS Companion Core 컴포넌트를 가져와서, 후처리 함수를 호출합니다.
	if (UGSCCoreComponent* TargetCoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(TargetActor))
	{
		TargetCoreComponent->PostGameplayEffectExecute(this, Data);
	}
}

void UGSCAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 부모 클래스의 복제 설정을 그대로 사용합니다.
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

float UGSCAttributeSetBase::GetClampMinimumValueFor(const FGameplayAttribute& Attribute)
{
	// 서브 클래스에서 다른 최소값을 사용하고자 한다면 오버라이드하여 구현할 수 있습니다.
	return 0.f;
}

const FGameplayTagContainer& UGSCAttributeSetBase::GetSourceTagsFromContext(const FGameplayEffectModCallbackData& Data)
{
	// Data 내 EffectSpec의 캡처된 소스 태그를 반환합니다.
	return *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
}

void UGSCAttributeSetBase::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, const float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty) const
{
	// 소유 AbilitySystemComponent를 가져옵니다.
	UAbilitySystemComponent* AbilitySystemComponent = GetOwningAbilitySystemComponent();
	if (!AbilitySystemComponent)
	{
		return;
	}

	// 현재 최대값과 현재 속성 값을 가져옵니다.
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	const float CurrentValue = AffectedAttribute.GetCurrentValue();

	// 만약 현재 최대값과 새 최대값이 다르고, 현재 최대값이 0보다 크면
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && CurrentMaxValue > 0.f)
	{
		// 현재 비율(Ratio)을 계산합니다.
		const float Ratio = CurrentValue / CurrentMaxValue;

		// 새 최대값에 비례하여 조정될 새 값을 계산합니다.
		const float NewValue = FMath::RoundToFloat(NewMaxValue * Ratio);

		// 디버깅 로그: 현재 값, 최대값, 새 최대값, 계산된 새 값, 비율 정보를 출력합니다.
		GSC_LOG(Verbose, TEXT("AdjustAttributeForMaxChange: CurrentValue: %f, CurrentMaxValue: %f, NewMaxValue: %f, NewValue: %f (Ratio: %f)"),
		        CurrentValue, CurrentMaxValue, NewMaxValue, NewValue, Ratio);
		GSC_LOG(Verbose, TEXT("AdjustAttributeForMaxChange: ApplyModToAttribute %s with %f"), *AffectedAttributeProperty.GetName(), NewValue);
		// AbilitySystemComponent를 통해 속성을 Override 방식으로 수정합니다.

		AbilitySystemComponent->ApplyModToAttribute(AffectedAttributeProperty, EGameplayModOp::Override, NewValue);
	}
}

void UGSCAttributeSetBase::GetExecutionDataFromMod(const FGameplayEffectModCallbackData& Data, FGSCAttributeSetExecutionData& OutExecutionData)
{
	// EffectSpec의 Context를 가져와 실행 데이터에 저장
	OutExecutionData.Context = Data.EffectSpec.GetContext();
	// 원본 Instigator ASC를 저장
	OutExecutionData.SourceASC = OutExecutionData.Context.GetOriginalInstigatorAbilitySystemComponent();
	// 소스 태그를 캡처하여 저장
	OutExecutionData.SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
	// Asset 태그들을 모두 저장
	Data.EffectSpec.GetAllAssetTags(OutExecutionData.SpecAssetTags);

	// 타겟 액터와 플레이어 컨트롤러, Pawn 등을 설정
	OutExecutionData.TargetActor = Data.Target.AbilityActorInfo->AvatarActor.IsValid() ? Data.Target.AbilityActorInfo->AvatarActor.Get() : nullptr;
	OutExecutionData.TargetController = Data.Target.AbilityActorInfo->PlayerController.IsValid() ? Data.Target.AbilityActorInfo->PlayerController.Get() : nullptr;
	OutExecutionData.TargetPawn = Cast<APawn>(OutExecutionData.TargetActor);
	// 타겟 액터에 부착된 GAS Companion Core 컴포넌트를 가져옴
	OutExecutionData.TargetCoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(OutExecutionData.TargetActor);

	// 소스 ASC가 유효한 경우 소스 액터, 컨트롤러, Pawn 등을 설정
	if (OutExecutionData.SourceASC && OutExecutionData.SourceASC->AbilityActorInfo.IsValid())
	{
		// 소스 액터 설정: Instigator의 AvatarActor가 유효하면
		if (OutExecutionData.SourceASC->AbilityActorInfo->AvatarActor.IsValid())
		{
			// 만약 Context에 EffectCauser가 있다면 이를 우선 사용
			if (OutExecutionData.Context.GetEffectCauser())
			{
				OutExecutionData.SourceActor = OutExecutionData.Context.GetEffectCauser();
			}
			else
			{
				// 그렇지 않으면 Instigator의 AvatarActor를 사용
				OutExecutionData.SourceActor = OutExecutionData.SourceASC->AbilityActorInfo->AvatarActor.IsValid()
					                               ? OutExecutionData.SourceASC->AbilityActorInfo->AvatarActor.Get()
					                               : nullptr;
			}
		}

		// 소스 컨트롤러와 Pawn 설정
		OutExecutionData.SourceController = OutExecutionData.SourceASC->AbilityActorInfo->PlayerController.IsValid()
			                                    ? OutExecutionData.SourceASC->AbilityActorInfo->PlayerController.Get()
			                                    : nullptr;
		OutExecutionData.SourcePawn = Cast<APawn>(OutExecutionData.SourceActor);
		// 소스 액터에 부착된 GAS Companion Core 컴포넌트 설정
		OutExecutionData.SourceCoreComponent = UGSCBlueprintFunctionLibrary::GetCompanionCoreComponent(OutExecutionData.SourceActor);
	}

	// SourceObject: EffectContext에서 SourceObject를 가져옴
	OutExecutionData.SourceObject = Data.EffectSpec.GetEffectContext().GetSourceObject();

	// DeltaValue 계산: 만약 Modifier 연산이 Additive라면, 그 값(변화량)을 저장합니다.
	OutExecutionData.DeltaValue = 0.f;
	if (Data.EvaluatedData.ModifierOp == EGameplayModOp::Type::Additive)
	{
		OutExecutionData.DeltaValue = Data.EvaluatedData.Magnitude;
	}
}
