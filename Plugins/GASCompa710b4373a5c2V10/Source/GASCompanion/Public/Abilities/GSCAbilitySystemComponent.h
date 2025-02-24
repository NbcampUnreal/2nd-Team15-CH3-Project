// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GSCTypes.h"
#include "Abilities/GSCAbilitySet.h"
#include "GSCAbilitySystemComponent.generated.h"

// 전방 선언: 입력 바인딩, 콤보 매니저, 입력 액션 등 관련 클래스
class UGSCAbilityInputBindingComponent;
class UGSCComboManagerComponent;
class UInputAction;

/**
 * @brief Ability를 부여할 때 필요한 입력 바인딩 정보를 담는 구조체
 *
 * 이 구조체는 ASC(Ability System Component) 초기화 시, 특정 Ability를 어떤 레벨(레벨), 어떤 입력 액션(InputAction),
 * 그리고 어떤 트리거 이벤트(TriggerEvent)에 바인딩할지 정의합니다. InputAction을 지정하지 않으면 입력 바인딩은 생략됩니다.
 */
USTRUCT(BlueprintType)
struct FGSCAbilityInputMapping
{
	GENERATED_BODY()

	/** @brief ASC에 부여할 Gameplay Ability 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	TSubclassOf<UGameplayAbility> Ability;

	/** @brief Ability를 부여할 레벨 (기본값 1) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	int32 Level = 1;

	/** @brief Ability 활성화 시 사용할 입력 액션 (미지정 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability)
	TObjectPtr<UInputAction> InputAction = nullptr;

	/**
	 * @brief Ability 활성화를 위한 Enhanced Input 트리거 이벤트 타입
	 *
	 * 일반적으로 버튼을 누르는 순간 한 번 실행되는 경우는 Started,
	 * 버튼을 누른 상태에서 매 프레임마다 실행되는 경우는 Triggered를 사용합니다.
	 *
	 * @warning Triggered는 매 틱마다 이벤트가 발생하므로, 단 한 번 실행되어야 하는 액션에는 Started 사용을 권장합니다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Ability, meta=(EditCondition = "InputAction != nullptr", EditConditionHides))
	EGSCAbilityTriggerEvent TriggerEvent = EGSCAbilityTriggerEvent::Started;
};

/**
 * @brief ASC 초기화 시 부여될 AttributeSet 정보를 담는 구조체
 *
 * 이 구조체는 AttributeSet 클래스를 지정하고, 선택적으로 그 값을 초기화할 DataTable(InitializationData)을 설정합니다.
 */
USTRUCT(BlueprintType)
struct FGSCAttributeSetDefinition
{
	GENERATED_BODY()

	/** @brief ASC에 부여할 AttributeSet 클래스 */
	UPROPERTY(EditAnywhere, Category=Attributes)
	TSubclassOf<UAttributeSet> AttributeSet;

	/** @brief AttributeSet 초기값을 설정할 DataTable (선택 사항) */
	UPROPERTY(EditAnywhere, Category=Attributes, meta = (RequiredAssetDataTags = "RowStructure=/Script/GameplayAbilities.AttributeMetaData"))
	TObjectPtr<UDataTable> InitializationData = nullptr;
};

/**
 * @brief ASC에 부여된 Ability와 해당 입력 액션 정보를 추적하기 위한 구조체
 *
 * AbilitySpecHandle과 AbilitySpec, 그리고 입력 액션을 함께 보관하여
 * 추후 입력 바인딩 해제 또는 Ability 제거 시 관리에 활용됩니다.
 */
USTRUCT()
struct FGSCMappedAbility
{
	GENERATED_BODY()

	/** @brief 부여된 Ability에 대한 핸들 */
	FGameplayAbilitySpecHandle Handle;

	/** @brief 부여된 Ability의 스펙 정보 */
	FGameplayAbilitySpec Spec;

	/** @brief Ability와 연결된 입력 액션 (런타임 transient) */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> InputAction;

	FGSCMappedAbility() : InputAction(nullptr)
	{
	}

	FGSCMappedAbility(const FGameplayAbilitySpecHandle& Handle, const FGameplayAbilitySpec& Spec, UInputAction* const InputAction)
		: Handle(Handle), Spec(Spec), InputAction(InputAction)
	{
	}
};

/**
 * @brief Ability가 부여될 때 호출되는 Multicast 델리게이트 (FGameplayAbilitySpec& 전달)
 */
DECLARE_MULTICAST_DELEGATE_OneParam(FGSCOnGiveAbility, FGameplayAbilitySpec&);

/**
 * @brief Ability Actor 정보(Owner/Avatar)가 초기화된 직후, 능력 및 속성이 부여된 후에 호출되는 델리게이트
 *
 * 클라이언트/서버 모두에서 여러 번(컴포넌트 초기화, 소유 액터 복제 등) 호출될 수 있습니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGSCOnInitAbilityActorInfo);

/**
 * @brief Revamped Ability System Component (버전 3.0.0)
 *
 * 블루프린트(또는 GameFeature)를 통해 쉽게 부착 가능하며,
 * ASC와 IAbilitySystemInterface를 C++에서 구현해야 하는 4.27에서도 최소한의 마이그레이션으로 사용 가능하도록 설계되었습니다.
 *
 * - ASC Initialization 시, 지정된 Ability, Attribute, GameplayEffect, Ability Set 등을 부여합니다.
 * - Ability 입력 바인딩 처리, Ability 재부여 여부(bResetAbilitiesOnSpawn, bResetAttributesOnSpawn) 등을 제어합니다.
 */
UCLASS(ClassGroup="GASCompanion", meta=(BlueprintSpawnableComponent))
class GASCOMPANION_API UGSCAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

	// =======================================================================
	//                        MEMBER FUNCTIONS
	// =======================================================================
public:
	// -----------------------
	// Engine Lifecycle
	// -----------------------

	/**
	 * @brief 컴포넌트가 BeginPlay 시점에 호출되는 함수
	 *
	 * 부모 클래스의 BeginPlay를 호출하고, Ability 활성/종료/실패 콜백을 등록합니다.
	 * 또한 초기 효과(GrantedEffects)를 적용합니다.
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief 객체가 제거(파괴)되기 전에 호출되는 함수
	 *
	 * - 등록된 델리게이트들을 제거하고,
	 * - 부여된 Attribute, Ability, Effect, Ability Set 등을 초기화합니다.
	 */
	virtual void BeginDestroy() override;


	// -----------------------
	// AbilitySystemComponent Overrides
	// -----------------------
public:
	/**
	 * @brief 소유자 및 아바타 액터 정보를 초기화하는 함수
	 *
	 * - 부모 클래스의 InitAbilityActorInfo를 호출한 뒤, 기본 Ability/Attribute/Ability Set을 부여하고,
	 * - OnInitAbilityActorInfo 델리게이트를 브로드캐스트합니다.
	 *
	 * @param InOwnerActor ASC 소유자 액터
	 * @param InAvatarActor ASC가 적용될 아바타 액터
	 */
	virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	/**
	 * @brief 입력이 눌렸을 때 호출되어, Ability를 활성화하는 함수
	 *
	 * - 만약 Ability가 콤보(Combo) 능력이라면 콤보 매니저를 통해 활성화를 처리합니다.
	 * - 일반 Ability라면 TryActivateAbility 등의 기본 로직을 통해 활성화합니다.
	 *
	 * @param InputID 입력 식별자
	 */
	virtual void AbilityLocalInputPressed(int32 InputID) override;

	/**
	 * @brief Ability가 부여될 때(ASC->GiveAbility) 자동 호출되는 함수
	 * 부모 클래스 로직 수행 후, 부여된 AbilitySpec을 OnGiveAbilityDelegate로 브로드캐스트하여 클라이언트 입력 바인딩 등 후속 처리를 가능하게 합니다.
	 * @param AbilitySpec 부여된 Ability의 스펙
	 */
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;


	// -----------------------
	// Ability Granting
	// -----------------------

	/**
	 * @brief [DEPRECATED] Ability를 ASC에 부여하기 위한 헬퍼 함수
	 *
	 * 5.0부터 BlueprintCallable이 제공되므로, 이제는 ASC->GiveAbility() 사용을 권장합니다.
	 *
	 * @param Ability 부여할 Gameplay Ability 클래스
	 * @param bRemoveAfterActivation Ability 활성화 후 제거할지 여부
	 * @return 부여된 Ability의 FGameplayAbilitySpecHandle
	 */
	UE_DEPRECATED(5.2, "Please use regular ASC->GiveAbility() method instead.")
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "GAS Companion|Abilities",
		meta=(DeprecatedFunction, DeprecationMessage="Please use GiveAbility() method of engine's UAbilitySystemComponent instead. Or the newly added GiveAbilityWithInput method of UGSCAbilityInputBindingComponent."))
	FGameplayAbilitySpecHandle GrantAbility(TSubclassOf<UGameplayAbility> Ability, bool bRemoveAfterActivation);

	/**
	 * @brief Ability Set을 ASC에 부여하여 Ability, Attribute, Effect, Owned Tag 등을 추가하는 함수
	 *
	 * - 서버와 클라이언트 모두에서 호출 가능해야 하며,
	 * - Pawn 초기화 시점(OnInitAbilityActorInfo, OnBeginPlay 등)에 호출됩니다.
	 * - 입력 바인딩을 사용하려면, 아바타 액터가 UGSCAbilityInputBindingComponent를 가지고 있어야 합니다.
	 *
	 * @param InAbilitySet 부여할 Ability Set
	 * @param OutHandle Ability Set을 제거할 때 사용하는 핸들
	 * @return 부여에 성공하면 true, 실패하면 false
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Ability Sets")
	bool GiveAbilitySet(const UGSCAbilitySet* InAbilitySet, FGSCAbilitySetHandle& OutHandle);

	/**
	 * @brief ASC에서 특정 Ability Set을 제거하는 함수
	 *
	 * - 이 함수는 Ability Set이 부여했던 Ability, Attribute, Effect, Owned Tag 등을 모두 제거합니다.
	 * - 서버와 클라이언트 모두에서 호출되도록 설계되어 있습니다.
	 * 
	 * @param InAbilitySetHandle 제거하려는 Ability Set의 핸들
	 * @return 제거에 성공하면 true, 실패하면 false
	 */
	UFUNCTION(BlueprintCallable, Category = "GAS Companion|Ability Sets")
	bool ClearAbilitySet(UPARAM(ref) FGSCAbilitySetHandle& InAbilitySetHandle);


	// -----------------------
	// Ability / Attribute Initialization
	// -----------------------

	/**
	 * @brief ASC 초기화(InitAbilityActorInfo) 시, 기본 Ability와 Attribute를 부여하는 함수
	 * - GrantedAbilities, GrantedAttributes를 확인하여 Ability와 Attribute를 부여합니다.
	 * - bResetAbilitiesOnSpawn, bResetAttributesOnSpawn 값에 따라 중복 부여를 막거나 재부여를 허용합니다.
	 * @param InOwnerActor ASC 소유자 액터
	 * @param InAvatarActor ASC가 적용될 아바타 액터
	 */
	virtual void GrantDefaultAbilitiesAndAttributes(AActor* InOwnerActor, AActor* InAvatarActor);

	/**
	 * @brief ASC 초기화(InitAbilityActorInfo) 시, 기본 Ability Set을 부여하는 함수
	 *
	 * - GrantedAbilitySets를 확인하여 Ability Set을 부여합니다.
	 * - Pawn/PlayerState에 따라 적절한 시점에 호출되어야 하며, 입력 바인딩에 필요한 Component 유무도 확인합니다.
	 *
	 * @param InOwnerActor ASC 소유자 액터
	 * @param InAvatarActor ASC가 적용될 아바타 액터
	 */
	virtual void GrantDefaultAbilitySets(AActor* InOwnerActor, AActor* InAvatarActor);


	// -----------------------
	// Ability Activation Callbacks
	// -----------------------

	/**
	 * @brief Ability 활성/종료/실패 시점에 등록된 콜백을 통해 Owning Character로 이벤트를 전달
	 *
	 * 주로 Ability 큐잉 로직과 연동합니다.
	 */
	virtual void OnAbilityActivatedCallback(UGameplayAbility* Ability);
	virtual void OnAbilityFailedCallback(const UGameplayAbility* Ability, const FGameplayTagContainer& Tags);
	virtual void OnAbilityEndedCallback(UGameplayAbility* Ability);


	// -----------------------
	// Helpers & Checks
	// -----------------------

	/**
	 * @brief 특정 Ability를 재부여할지 여부를 판단하는 함수
	 *
	 * - bResetAbilitiesOnSpawn가 false인 경우, 이미 부여된 Ability와 동일 타입/레벨이면 재부여를 방지합니다.
	 *
	 * @param InAbility 부여하려는 Ability 클래스
	 * @param InLevel Ability 레벨 (기본값 1)
	 * @return 재부여가 필요하면 true, 이미 존재하면 false
	 */
	virtual bool ShouldGrantAbility(TSubclassOf<UGameplayAbility> InAbility, const int32 InLevel = 1);

	/**
	 * @brief 특정 Ability Set을 재부여할지 여부를 판단하는 함수
	 *
	 * - 이전에 같은 Ability Set이 부여되었으면 재부여를 막습니다 (단, PlayerState 기반 ASC일 경우 다시 부여)
	 *
	 * @param InAbilitySet 부여하려는 Ability Set
	 * @return 재부여가 필요하면 true, 이미 존재하면 false
	 */
	virtual bool ShouldGrantAbilitySet(const UGSCAbilitySet* InAbilitySet) const;

	/**
	 * @brief 현재 ASC 소유자 액터가 PlayerState인지 여부를 확인
	 *
	 * @return PlayerState 소유자이면 true, 아니면 false
	 */
	bool IsPlayerStateOwner() const;

protected:
	// -----------------------
	// Internal Utility
	// -----------------------

	/** @brief BeginPlay 시 부여해야 할 초기 효과(GrantedEffects)를 적용하는 함수 */
	void GrantStartupEffects();

	/**
	 * @brief Pawn의 컨트롤러가 변경될 때, AbilityActorInfo를 재초기화하는 함수
	 *
	 * 주로 PlayerController 교체 시, ASC 내부 캐싱 정보(AnimInstance 등)를 다시 세팅합니다.
	 *
	 * @param Pawn 해당 Pawn
	 * @param NewController 새 컨트롤러
	 */
	UFUNCTION()
	void OnPawnControllerChanged(APawn* Pawn, AController* NewController);

	/**
	 * @brief Ability가 부여된 직후(서버, 클라이언트 모두) 클라이언트 측 입력 바인딩을 설정하는 함수
	 *
	 * @param AbilitySpec 부여된 Ability의 스펙
	 * @param InputComponent 입력 바인딩 컴포넌트
	 * @param InputAction 입력 액션
	 * @param TriggerEvent 입력 트리거 이벤트 타입
	 * @param NewAbilitySpec 새로 부여된 Ability의 스펙
	 */
	virtual void HandleOnGiveAbility(FGameplayAbilitySpec& AbilitySpec,
	                                 UGSCAbilityInputBindingComponent* InputComponent,
	                                 UInputAction* InputAction,
	                                 EGSCAbilityTriggerEvent TriggerEvent,
	                                 FGameplayAbilitySpec NewAbilitySpec);


	// =======================================================================
	//                        MEMBER VARIABLES
	// =======================================================================

public:
	// -----------------------
	// Ability & Attribute Definitions
	// -----------------------

	/** @brief ASC 초기화 시 자동으로 부여할 Ability 목록 (입력 바인딩 포함 가능) */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Abilities")
	TArray<FGSCAbilityInputMapping> GrantedAbilities;

	/** @brief ASC 초기화 시 자동으로 부여할 AttributeSet 목록 */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Abilities")
	TArray<FGSCAttributeSetDefinition> GrantedAttributes;

	/** @brief ASC 초기화 시 적용할 GameplayEffect 목록 */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Abilities")
	TArray<TSubclassOf<UGameplayEffect>> GrantedEffects;

	/** @brief ASC 초기화 시 부여할 Ability Set 목록 */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Abilities")
	TArray<TSoftObjectPtr<UGSCAbilitySet>> GrantedAbilitySets;

	/**
	 * @brief InitAbilityActorInfo 완료 후, Ability와 Attribute가 부여된 직후 호출되는 이벤트
	 *
	 * - 서버/클라이언트 모두에서 여러 번(컴포넌트 초기화 후 또는 소유 액터 복제 후) 호출될 수 있습니다.
	 * - ASC가 Pawn 혹은 PlayerState에 있는지에 따라 호출 시점이 달라질 수 있습니다.
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS Companion|Abilities")
	FGSCOnInitAbilityActorInfo OnInitAbilityActorInfo;

	/**
	 * @brief 죽음/리스폰 또는 소유권 이전 시, Ability가 재부여될지 여부
	 *
	 * - false로 설정하면, InitAbilityActorInfo가 처음 호출될 때만 Ability를 부여하고 이후엔 부여하지 않습니다.
	 * - ASC가 PlayerState에 있을 경우 이 값이 false가 되는 것이 일반적입니다.
	 * - Pawn에 ASC가 있을 때만 true로 설정하여 Ability를 재부여하도록 사용할 수 있습니다.
	 * - 기본값은 true입니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Abilities")
	bool bResetAbilitiesOnSpawn = true;

	/**
	 * @brief 죽음/리스폰 또는 소유권 이전 시, Attribute가 재부여될지 여부
	 *
	 * - false로 설정하면, InitAbilityActorInfo가 처음 호출될 때만 Attribute를 부여합니다.
	 * - ASC가 PlayerState에 있을 경우 false가 기본 동작입니다.
	 * - true로 설정하면 Attribute를 재초기화(기본값으로)하고자 할 때 사용합니다.
	 * - 기본값은 true입니다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "GAS Companion|Abilities")
	bool bResetAttributesOnSpawn = true;

	/** @brief Ability가 부여되었을 때 호출되는 델리게이트 (멀티캐스트) */
	FGSCOnGiveAbility OnGiveAbilityDelegate;

protected:
	// -----------------------
	// Runtime Data
	// -----------------------

	/** @brief 부여된 Ability 목록(핸들과 스펙, 입력 액션) */
	UPROPERTY(transient)
	TArray<FGSCMappedAbility> AddedAbilityHandles;

	/** @brief 부여된 AttributeSet 인스턴스 목록 */
	UPROPERTY(transient)
	TArray<TObjectPtr<UAttributeSet>> AddedAttributes;

	/** @brief BeginPlay 시 적용된 Startup Effects의 핸들 목록 */
	UPROPERTY(transient)
	TArray<FActiveGameplayEffectHandle> AddedEffects;

	/** @brief 부여된 Ability Set 목록(핸들) */
	UPROPERTY(transient)
	TArray<FGSCAbilitySetHandle> AddedAbilitySets;

	/** @brief 클라이언트 입력 바인딩을 위해 OnGiveAbility에 등록된 델리게이트 핸들 목록 */
	TArray<FDelegateHandle> InputBindingDelegateHandles;

	/** @brief 콤보 능력 활성화를 위한 콤보 매니저 컴포넌트 (아바타에 있으면 캐싱) */
	UPROPERTY()
	TObjectPtr<UGSCComboManagerComponent> ComboComponent;
};
