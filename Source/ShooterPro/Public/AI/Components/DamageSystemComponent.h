#pragma once

#include "CoreMinimal.h"
#include "AI/EnemyAITypes.h"
#include "Components/ActorComponent.h"
#include "DamageSystemComponent.generated.h"

/*=============================================================================
    Delegate Declarations
    - FOnDeath:   액터가 사망할 때 호출됨.
    - FOnBlocked: 데미지 블록 시, 파리 가능 여부와 데미지를 유발한 액터 정보를 전달.
    - FOnDamageResponse: 데미지 처리 후 추가 행동(예: 인터럽트)을 요청할 때 호출.
=============================================================================*/
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBlocked, bool, bCanBeParried, AActor*, DamageCauser);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageResponse, EDamageResponse, DamageResponse, AActor*, DamageCauser);

/*=============================================================================
    UDamageSystemComponent Class
    - 액터의 데미지 처리, 체력 회복, 공격 토큰 관리 및 관련 이벤트 전달을 담당.
=============================================================================*/
UCLASS(Blueprintable, BlueprintType, ClassGroup=("AI"), meta=(BlueprintSpawnableComponent))
class SHOOTERPRO_API UDamageSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDamageSystemComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//-------------------------------------------------------------------------
	// Damage Handling & Healing Functions
	//-------------------------------------------------------------------------
public:
	/**
	 * @brief 데미지 처리 함수
	 *
	 * 주어진 데미지 정보를 바탕으로 액터에 데미지를 입힙니다.
	 * - 액터가 사망하지 않았고, 무적 상태가 아니거나 무적 상태에도 데미지가 적용되어야 하는 경우에만 처리합니다.
	 * - 블록 상태인 경우, OnBlocked 델리게이트를 호출 후 데미지를 적용하지 않습니다.
	 * - 체력이 0 이하가 되면 사망 처리 후 OnDeath 델리게이트를 호출합니다.
	 * - 데미지에 따른 행동 인터럽트가 필요하면 OnDamageResponse 델리게이트를 호출합니다.
	 * 
	 * @param DamageInfo 데미지 관련 정보 (금액, 블록 가능 여부, 인터럽트 플래그 등)
	 * @param DamageCauser 데미지를 유발한 액터
	 * @return 데미지 적용 성공 시 true, 블록 등으로 적용되지 않으면 false
	 */
	UFUNCTION(BlueprintCallable, Category="DamageSystemComponent")
	bool TakeDamage(FDamageInfo DamageInfo, AActor* DamageCauser);

	/**
	 * @brief 체력 회복 함수
	 * 지정된 Amount만큼 체력을 회복하며, 회복 후 최대 체력(MaxHealth)을 초과하지 않도록 합니다.
	 * **주의:** 액터가 살아있는 경우(bIsDead == false)는 회복되지 않습니다.
	 * @param Amount 회복할 체력 값
	 * @return 회복 후의 체력 값
	 */
	UFUNCTION(BlueprintCallable, Category="DamageSystemComponent")
	float Heal(float Amount);

	//-------------------------------------------------------------------------
	// Attack Token Management Functions
	//-------------------------------------------------------------------------
public:
	/**
	 * @brief 공격 토큰 예약 함수
	 * 공격 실행 시 사용할 토큰을 예약합니다. 예약할 토큰이 충분할 경우 해당 수량만큼 차감합니다.
	 * @param Amount 예약할 토큰 수
	 * @return 예약 성공 시 true, 부족할 경우 false
	 */
	UFUNCTION(BlueprintCallable, Category="DamageSystemComponent")
	bool ReserveAttackToken(int Amount);

	/**
	 * @brief 공격 토큰 반환 함수
	 * 사용 후 반환된 공격 토큰을 다시 추가합니다.
	 * @param Amount 반환할 토큰 수
	 */
	UFUNCTION(BlueprintCallable, Category="DamageSystemComponent")
	void ReturnAttackToken(int Amount);


	/*=========================================================================
	    Public Member Variables
	    - 멤버 변수는 상태 값 및 이벤트 델리게이트 등으로 하단에 그룹화.
	==========================================================================*/

	//-------------------------------------------------------------------------
	// State Variables
	//-------------------------------------------------------------------------
public:
	/**
	 * @brief 무적 상태 여부
	 * true일 경우, (특정 조건 하에서) 데미지를 받지 않습니다.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DamageSystemComponent")
	bool bIsInvincible = false;

	/**
	 * @brief 사망 상태 여부
	 * true이면 액터가 사망한 상태입니다.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DamageSystemComponent")
	bool bIsDead = false;

	/**
	 * @brief 인터럽트 가능 여부
	 * 데미지로 인해 행동이 인터럽트될 수 있는지 여부를 나타냅니다.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DamageSystemComponent")
	bool bIsInterruptible = true;

	/**
	 * @brief 블록 상태 여부
	 * 현재 데미지 블록 중인지 여부를 나타냅니다.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DamageSystemComponent")
	bool bIsBlocking = false;

	/**
	 * @brief 공격 토큰 수
	 * 공격 실행에 사용 가능한 토큰의 개수입니다.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DamageSystemComponent")
	int32 AttackTokensCount = 1;

	/**
	 * @brief 현재 체력
	 * 액터의 현재 체력 값입니다.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DamageSystemComponent")
	float Health = 50.0f;

	/**
	 * @brief 최대 체력
	 * 액터가 가질 수 있는 최대 체력 값입니다.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="DamageSystemComponent")
	float MaxHealth = 100.0f;


	//-------------------------------------------------------------------------
	// Delegate Events
	//-------------------------------------------------------------------------
public:
	/**
	 * @brief 사망 델리게이트
	 * 액터가 사망할 때 호출됩니다.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="DamageSystemComponent|Delegate")
	FOnDeath OnDeath;

	/**
	 * @brief 블록 델리게이트
	 * 데미지가 블록될 때, 파리 가능 여부와 데미지를 유발한 액터 정보를 전달합니다.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="DamageSystemComponent|Delegate")
	FOnBlocked OnBlocked;

	/**
	 * @brief 데미지 응답 델리게이트
	 * 데미지 처리 후 추가적인 행동 인터럽트 등이 필요한 경우 호출됩니다.
	 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="DamageSystemComponent|Delegate")
	FOnDamageResponse OnDamageResponse;
};
