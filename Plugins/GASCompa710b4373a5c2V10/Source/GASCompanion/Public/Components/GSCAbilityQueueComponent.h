// Copyright 2021 Mickael Daniel. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "GSCAbilityQueueComponent.generated.h"

/** Forward Declarations */
class UAbilitySystemComponent;
class UGameplayAbility;

/**
 * @class UGSCAbilityQueueComponent
 * @brief 이 컴포넌트는 액터(주로 Pawn)에 부착되어 '능력 큐(Ability Queue)' 기능을 제공합니다.
 * 
 * - '능력 큐'를 통해 능력 활성화 실패 시 재시도를 위한 큐에 능력을 임시 저장할 수 있습니다.
 * - 큐를 열었다가(OpenAbilityQueue) 닫는(CloseAbilityQueue) 로직을 제어합니다.
 * - 특정 능력 목록만 큐에 등록할 수 있도록 제한하거나, 모든 능력을 허용할 수도 있습니다.
 * 
 * @note 
 * - 현재 구현에서는 GSCAbilityInputBinding을 통해 바인딩된 능력들과 직접 연동되지 않습니다(수동 활성화 필요).
 * - 큐가 열려있을 때만 능력이 등록(실패 시)되며, 큐가 닫혀 있으면 등록되지 않습니다.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=("GASCompanion"), meta=(BlueprintSpawnableComponent))
class GASCOMPANION_API UGSCAbilityQueueComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	//===========================================================================
	// 1. 생성자(Constructors) & 엔진 오버라이드(Overrides)
	//===========================================================================

	/** 
	 * @brief 기본 생성자
	 * @details 이 컴포넌트의 기본 속성과 상태 변수를 초기화합니다.
	 */
	UGSCAbilityQueueComponent();

protected:
	/**
	 * @brief 게임이 시작될 때 호출되는 함수
	 * @details Pawn이나 다른 액터에 이 컴포넌트가 부착된 후, 초기 설정에 사용됩니다.
	 */
	virtual void BeginPlay() override;

public:
	//===========================================================================
	// 2. 오너(Owner) 관련 초기화 함수
	//===========================================================================

	/**
	 * @brief 소유자 Pawn 및 연결된 AbilitySystemComponent 정보를 설정합니다.
	 * @details Pawn이 아닌 액터라면 설정이 무효화될 수 있습니다.
	 */
	void SetupOwner();

	//===========================================================================
	// 3. 능력 큐 열기/닫기 & 설정 관련 함수
	//===========================================================================

	/**
	 * @brief 능력 큐를 열어(bAbilityQueueOpened = true) 실패한 능력 등록을 허용합니다.
	 */
	void OpenAbilityQueue();

	/**
	 * @brief 능력 큐를 닫아(bAbilityQueueOpened = false) 더 이상 등록되지 않도록 합니다.
	 */
	void CloseAbilityQueue();

	/**
	 * @brief 큐에 등록 가능한 능력 클래스를 설정합니다.
	 * @param AllowedAbilities 큐에서 허용할 능력 클래스 배열
	 */
	void UpdateAllowedAbilitiesForAbilityQueue(TArray<TSubclassOf<UGameplayAbility>> AllowedAbilities);

	/**
	 * @brief 모든 능력을 큐에 등록할 수 있도록 허용할지 여부를 설정합니다.
	 * @param bAllowAllAbilities true인 경우 모든 능력이 큐에 등록 가능
	 */
	void SetAllowAllAbilitiesForAbilityQueue(bool bAllowAllAbilities);

	//===========================================================================
	// 4. 능력 큐 상태 확인 함수
	//===========================================================================

	/**
	 * @brief 현재 능력 큐가 열려 있는지 여부를 반환합니다.
	 * @return 큐가 열려 있다면 true, 그렇지 않다면 false
	 */
	bool IsAbilityQueueOpened() const;

	/**
	 * @brief 모든 능력이 큐에 등록되도록 허용하는지 여부를 반환합니다.
	 * @return 모든 능력이 허용되면 true, 아니라면 false
	 */
	bool IsAllAbilitiesAllowedForAbilityQueue() const;

	/**
	 * @brief 현재 큐에 등록되어 있는 능력을 반환합니다.
	 * @return 큐에 있는 UGameplayAbility 포인터 (없다면 nullptr)
	 */
	const UGameplayAbility* GetCurrentQueuedAbility() const;

	/**
	 * @brief 큐에 등록할 수 있도록 설정된 허용 능력 클래스들의 배열을 반환합니다.
	 * @return 허용되는 능력 클래스 배열
	 */
	TArray<TSubclassOf<UGameplayAbility>> GetQueuedAllowedAbilities() const;

	//===========================================================================
	// 5. 이벤트 핸들러 & 능력 큐 로직 함수
	//===========================================================================

	/**
	 * @brief 소유자가 활성화한 능력이 종료되었을 때 호출됩니다.
	 * @param InAbility 종료된 능력
	*/
	void OnAbilityEnded(const UGameplayAbility* InAbility);

	/**
	 * @brief 소유자의 능력 활성화가 실패했을 때 호출됩니다.
	 * @param Ability 실패한 능력
	 * @param ReasonTags 실패 원인을 나타내는 태그 컨테이너
	*/
	void OnAbilityFailed(const UGameplayAbility* Ability, const FGameplayTagContainer& ReasonTags);

protected:
	//===========================================================================
	// 6. 내부 동작 함수
	//===========================================================================

	/**
	 * @brief 능력 큐 관련 내부 변수를 초기 상태로 재설정합니다.
	*/
	virtual void ResetAbilityQueueState();

	/**
	 * @brief 디버그 위젯에 현재 허용된 능력 목록을 업데이트합니다.
	*/
	virtual void UpdateDebugWidgetAllowedAbilities();

	//===========================================================================
	// 7. 멤버 변수(상태/참조)
	//===========================================================================

public:
	/**
	 * @brief 이 컴포넌트를 소유하는 Pawn(캐릭터 등)
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Components")
	TObjectPtr<APawn> OwnerPawn;

	/**
	 * @brief 소유 Pawn에 부착된 AbilitySystemComponent
	 */
	UPROPERTY(BlueprintReadOnly, Category = "GAS Companion|Components")
	TObjectPtr<UAbilitySystemComponent> OwnerAbilitySystemComponent;

	/**
	 * @brief 이 컴포넌트의 능력 큐 시스템이 활성화되어 있는지 여부 
	 * @details 비활성화(true->false) 시 큐 기능 자체가 동작하지 않습니다.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS Companion|Ability Queue System")
	bool bAbilityQueueEnabled = true;

protected:
	/** @brief 현재 능력 큐가 열려 있는지 여부 */
	bool bAbilityQueueOpened = false;

	/** @brief 모든 능력을 큐에 등록 가능하게 할지 여부 */
	bool bAllowAllAbilitiesForAbilityQueue = false;

	/**
	 * @brief 현재 큐에 등록된 능력 
	 * @details 재시도 활성화를 위해 저장됨
	 */
	UPROPERTY()
	TObjectPtr<UGameplayAbility> QueuedAbility;

	/**
	 * @brief 큐에 등록 가능하도록 허용된 능력 클래스 목록 
	 * @details 모든 능력을 허용하는 경우(bAllowAllAbilitiesForAbilityQueue=true) 이 목록은 무시됩니다.
	 */
	TArray<TSubclassOf<UGameplayAbility>> QueuedAllowedAbilities;
};
