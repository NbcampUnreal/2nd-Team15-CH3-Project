#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Components/ActorComponent.h"
#include "GSCComboManagerComponent.generated.h"

/**
 * Forward Declarations
 * - UGSCComboManagerComponent에서 참조하는 클래스들에 대해 미리 선언해 둡니다.
 */
class UGSCCoreComponent;
class UAbilitySystemComponent;
class UGSCGameplayAbility;
class ACharacter;

/**
 * @class UGSCComboManagerComponent
 * @brief 콤보(Combo) 시스템을 관리하기 위한 컴포넌트
 * 
 * UGSCCoreComponent와 함께 사용되며, 캐릭터가 사용하는 연속 타격(콤보) 능력을 관리합니다.
 * - 콤보 지수(ComboIndex)를 관리하고, 콤보 윈도우 열림/닫힘을 확인합니다.
 * - 다음 콤보를 이어갈지 여부 등을 제어합니다.
 * - 능력(UGSCGameplayAbility) 활성화 로직과 네트워크 동기화 과정을 포함합니다.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=("GASCompanion"), meta=(BlueprintSpawnableComponent))
class GASCOMPANION_API UGSCComboManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * @brief 컴포넌트의 기본 생성자
	 * @details Tick 가능 여부, 복제(Replication) 설정 등을 초기화합니다.
	 */
	UGSCComboManagerComponent();


	/**
	 * @brief 서버와 클라이언트 간에 이 컴포넌트의 멤버 변수를 어떻게 복제(Replicate)할지 설정합니다.
	 * @param OutLifetimeProps 멤버 변수를 복제하기 위한 속성 정보 배열
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * @brief 오너(Owner)를 캐릭터로 설정하고, 필요 컴포넌트/시스템을 초기화합니다.
	 * @details BeginPlay나 OnRegister에서 호출되어, OwningCharacter와 OwnerCoreComponent를 세팅합니다.
	 */
	void SetupOwner();

	/**
	 * @brief 현재 활성화된 콤보 능력을 반환합니다.
	 * @return 현재 콤보 능력(UGameplayAbility) 포인터 (없으면 nullptr)
	 */
	UGameplayAbility* GetCurrentActiveComboAbility() const;

	/**
	 * @brief 콤보 윈도우가 열려 있을 때 콤보 지수(ComboIndex)를 1 증가시킵니다.
	 * @details bComboWindowOpened가 true인 경우에만 증가합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="GAS Companion|Combat")
	void IncrementCombo();

	/**
	 * @brief 콤보 지수(ComboIndex)를 0으로 되돌립니다.
	 * @details 콤보를 초기 상태로 만드는 로직에서 사용합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="GAS Companion|Combat")
	void ResetCombo();

	/**
	 * @brief 콤보 시스템의 핵심: 콤보 능력 활성화 로직
	 * @param AbilityClass 활성화할 능력 클래스 (보통 UGSCGameplayAbility 기반)
	 * @param bAllowRemoteActivation 원격(네트워크) 활성화를 허용할지 여부
	 * @details 콤보가 이미 재생 중인지, 혹은 새로 활성화해야 하는지 판단하고, 서버-클라이언트 간 동기화를 수행합니다.
	 */
	UFUNCTION(BlueprintCallable, Category="GAS Companion|Combat")
	void ActivateComboAbility(TSubclassOf<UGSCGameplayAbility> AbilityClass, bool bAllowRemoteActivation = true);

	/**
	 * @brief 콤보 지수(ComboIndex)를 설정합니다.
	 * @param InComboIndex 설정할 콤보 인덱스
	 * @details 서버 권한 여부를 확인하여, 서버가 아닌 경우에는 RPC를 통해 서버에 동기화합니다.
	 */
	void SetComboIndex(int32 InComboIndex);

	/**
	 * @brief 이 컴포넌트를 소유한 액터가 네트워크 권한을 가지고 있는지(Authority) 여부를 반환합니다.
	 * @return 네트워크 권한이 있으면 true, 그렇지 않으면 false
	 */
	virtual bool IsOwnerActorAuthoritative() const;

protected:
	//~Begin UActorComponent interface
	/**
	 * @brief 게임 시작 시(Play) 호출되는 함수
	 * @details 오버라이드하여 콤보 관련 초기화(SetupOwner 등)을 수행합니다.
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief 컴포넌트가 등록될 때 호출되는 함수
	 * @details 콤포넌트가 월드에 배치될 때, 네트워크 역할 등을 캐싱합니다.
	 */
	virtual void OnRegister() override;
	//~End UActorComponent interface

	/**
	 * @brief 서버에서 콤보 능력을 활성화하는 함수 (RPC)
	 * @param AbilityClass 활성화할 능력 클래스
	 * @param bAllowRemoteActivation 원격 활성화 허용 여부
	 */
	UFUNCTION(Server, Reliable)
	void ServerActivateComboAbility(TSubclassOf<UGSCGameplayAbility> AbilityClass, bool bAllowRemoteActivation = true);

	/**
	 * @brief 모든 클라이언트에게 콤보 능력을 활성화하라고 알리는 함수 (Multicast RPC)
	 * @param AbilityClass 활성화할 능력 클래스
	 * @param bAllowRemoteActivation 원격 활성화 허용 여부
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastActivateComboAbility(TSubclassOf<UGSCGameplayAbility> AbilityClass, bool bAllowRemoteActivation = true);

	/**
	 * @brief 콤보 능력을 내부적으로 실제 활성화하는 함수
	 * @param AbilityClass 활성화할 능력 클래스
	 * @param bAllowRemoteActivation 원격 활성화 허용 여부
	 * @details 서버와 클라이언트에서 공통으로 호출될 로직입니다.
	 */
	void ActivateComboAbilityInternal(TSubclassOf<UGSCGameplayAbility> AbilityClass, bool bAllowRemoteActivation = true);

	/**
	 * @brief 콤보 지수를 서버에 설정하도록 요청하는 함수 (RPC)
	 * @param InComboIndex 설정할 콤보 인덱스
	 */
	UFUNCTION(Server, Reliable)
	void ServerSetComboIndex(int32 InComboIndex);

	/**
	 * @brief 서버에서 변경된 콤보 지수를 모든 클라이언트에 반영하는 함수 (Multicast RPC)
	 * @param InComboIndex 설정할 콤보 인덱스
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSetComboIndex(int32 InComboIndex);

private:
	/**
	 * @brief 이 컴포넌트의 네트워크 상태(권한 유/무)를 캐싱합니다.
	 * @details BeginPlay나 OnRegister에서 호출되며, bCachedIsNetSimulated를 설정합니다.
	 */
	void CacheIsNetSimulated();

public:
	/**
	  * @brief 이 컴포넌트를 소유하는 ACharacter 객체
	  * @details 콤보 기능은 주로 캐릭터 전용이므로, 소유자를 캐릭터로 캐스팅하여 보관합니다.
	  *          에디터나 런타임 상황에 따라 유효하지 않을 수 있으므로, nullptr 체크가 필요합니다.
	 */
	UPROPERTY()
	TObjectPtr<ACharacter> OwningCharacter;

	/**
	 * @brief 소유 캐릭터에 있는 UGSCCoreComponent
	 * @details 콤보 능력 활성화 시, UGSCCoreComponent를 통해 실제 AbilitySystem과 상호작용합니다.
	 */
	UPROPERTY()
	TObjectPtr<UGSCCoreComponent> OwnerCoreComponent;

	/**
	 * @brief 기본 근접 공격(메인 콤보)에 활용될 능력 클래스
	 * @details MeleeBaseAbility로 지정된 능력을 기준으로 현재 활성화된 콤보 능력을 찾아냅니다.
	 */
	TSubclassOf<UGSCGameplayAbility> MeleeBaseAbility;

	/**
	 * @brief 현재 활성화되어 있는 콤보의 '콤보 지수'
	 * @details 0부터 시작하여, 콤보가 이어질 때마다 +1씩 증가됩니다.
	 */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GAS Companion|Combo")
	int32 ComboIndex = 0;

	/**
	 * @brief 콤보 윈도우가 열렸는지 여부
	 * @details true인 경우, 다음 콤보 동작을 큐잉할 수 있습니다.  
	 *          보통 콤보 몽타주가 재생되는 특정 시점에 열리고, 종료되면 닫힙니다.
	 */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GAS Companion|Combo")
	bool bComboWindowOpened = false;

	/**
	 * @brief 콤보 능력을 큐에 넣어야 하는지(트리거 대기 상태) 여부
	 * @details 현재 콤보가 재생 중일 때, 다음 공격을 예약할지(false -> true) 제어하는 플래그로 쓰일 수 있습니다.
	 */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GAS Companion|Combo")
	bool bShouldTriggerCombo = false;

	/**
	 * @brief 다음 콤보 몽타주를 트리거해야 하는지 여부
	 * @details bShouldTriggerCombo와 유사한 목적으로, 필요 시 개발자가 이를 통해 콤보 진행을 제어할 수 있습니다.
	 */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GAS Companion|Combo")
	bool bRequestTriggerCombo = false;

	/**
	 * @brief 다음 콤보 능력이 활성화되었는지 여부
	 * @details bShouldTriggerCombo나 bRequestTriggerCombo 등이 설정된 뒤, 실제로 능력이 성공적으로 활성화되었는지 표시할 수 있습니다.
	 */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GAS Companion|Combo")
	bool bNextComboAbilityActivated = false;

protected:
	/**
	 * @brief Net Role을 캐싱해놓은 플래그 (NetSimulated, NetAuthority 등)
	 * @details 서버(권한 있음) / 클라이언트(권한 없음)를 구분할 때 사용합니다.
	 */
	UPROPERTY()
	bool bCachedIsNetSimulated;
};
