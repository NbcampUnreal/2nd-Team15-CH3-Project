#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/Interface_Damageable.h"
#include "Interfaces/Interface_EnemyAI.h"
#include "ModularGameplayActors/GSCModularCharacter.h"
#include "EnemyAIBase.generated.h"

/**
 * 전방 선언
 */
class UAttackComponent;
class UWidgetComponent;
class AEnemyAIController;
class UDamageSystemComponent;
class UAICollisionComponent;
class UAIBehaviorsComponent;
class UBehaviorTree;
class APatrolPath;

/**
 * 공격 종료, 무기 장착/해제, 블록 종료 시 발생하는 델리게이트
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnd);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponEquipped);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponUnEquipped);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlockEnded);

/**
 * AEnemyAIBase 클래스
 * 
 * AI 적 캐릭터의 기본 동작을 담당하며, 피해 처리 및 AI 관련 인터페이스를 구현한다.
 * IInterface_Damageable, IInterface_EnemyAI 인터페이스를 상속받아 다양한 액션(공격, 이동, 블록 등)을 처리함.
 */
UCLASS()
class SHOOTERPRO_API AEnemyAIBase : public AGSCModularCharacter, public IInterface_Damageable, public IInterface_EnemyAI
{
	GENERATED_BODY()

public:
	/** 생성자 */
	AEnemyAIBase();

protected:
	/** 게임 시작 시 또는 스폰 후 최초 호출 */
	virtual void BeginPlay() override;

public:
	/** 매 프레임마다 호출되는 함수 */
	virtual void Tick(float DeltaSeconds) override;

protected:
	/** 캐릭터 사망 시 호출되는 함수 */
	UFUNCTION()
	void Die();

	/**
	 * 피해 반응 처리 함수
	 * @param DamageResponse - 피해 반응 타입 (HitReaction, Stagger 등)
	 * @param DamageCauser - 피해를 준 액터
	 */
	UFUNCTION()
	void HitResponse(EDamageResponse DamageResponse, AActor* DamageCauser);

	/**
	 * 공격이 블록되었을 때 호출되는 함수
	 * @param bCanBeParried - 패링 가능 여부
	 * @param DamageCauser - 공격을 수행한 액터
	 */
	UFUNCTION()
	void OnBlocked(bool bCanBeParried, AActor* DamageCauser);

protected:
	/**
	 * 몽타주 재생 종료 시 호출되는 함수
	 * OnCompleted / OnInterrupted 모두 이 함수에서 처리한다.
	 * @param AnimMontage - 재생된 애님 몽타주
	 * @param bInterrupted - 중단되었는지 여부 (true: 중단, false: 정상 종료)
	 */
	UFUNCTION()
	void OnHitMontageEnded(UAnimMontage* AnimMontage, bool bInterrupted);

public:
	/** 블록 시작 함수 */
	UFUNCTION(BlueprintCallable,Category="AI Base")
	void StartBlock();

	/** 블록 시도 (BlockChance 확률에 따라 실행) */
	UFUNCTION(BlueprintCallable,Category="AI Base")
	void TryToBlock();

	/** 블록 종료 함수 */
	UFUNCTION(BlueprintCallable,Category="AI Base")
	void EndBlock();

	//=============================================================================
	// Interface_Damageable 구현 (피해, 체력, 공격 토큰 등)
	//=============================================================================
public:
	/**
	 * @brief ReturnAttackToken: 예약된 공격 토큰을 반환한다.
	 * @param Amount - 반환할 토큰 수
	 */
	virtual void ReturnAttackToken_Implementation(int Amount) override;

	/**
	 * @brief SetIsInterruptible: 공격 중 인터럽트 가능 여부 설정
	 * @param bNewIsInterruptible - 새로운 인터럽트 가능 여부
	 */
	virtual void SetIsInterruptible_Implementation(bool bNewIsInterruptible) override;

	/**
	 * @brief SetIsInvincible: 무적 여부 설정
	 * @param bNewIsInvincible - 새로운 무적 여부
	 */
	virtual void SetIsInvincible_Implementation(bool bNewIsInvincible) override;

	/**
	 * @brief GetTeamNumber: 팀 번호 반환
	 * @return 팀 번호 (int32)
	 */
	virtual int32 GetTeamNumber_Implementation() override;
	virtual bool ReserveAttackToken_Implementation(int Amount) override;
	virtual bool IsAttacking_Implementation() override;
	virtual bool IsDead_Implementation() override;

	/**
	 * @brief TakeDamage: 피해 처리 함수
	 * @param DamageInfo - 피해 정보 구조체
	 * @param DamageCauser - 피해를 준 액터
	 * @return 실제 적용된 피해량 (bool: 처리 성공 여부)
	 */
	virtual bool TakeDamage_Implementation(FDamageInfo DamageInfo, AActor* DamageCauser) override;

	/**
	 * @brief Heal: 체력 회복 처리
	 * @param Amount - 회복량
	 * @return 실제 회복된 체력량 (float)
	 */
	virtual float Heal_Implementation(float Amount) override;

	/**
	 * @brief GetMaxHealth: 최대 체력 반환
	 * @return 최대 체력 (float)
	 */
	virtual float GetMaxHealth_Implementation() override;

	/**
	 * @brief GetCurrentHealth: 현재 체력 반환
	 * @return 현재 체력 (float)
	 */
	virtual float GetCurrentHealth_Implementation() override;

	//=============================================================================
	// Interface_EnemyAI 구현 (이동, 공격, 순찰 등)
	//=============================================================================
public:
	virtual void JumpToDestination_Implementation(FVector NewDestination) override;

	/**
	 * @brief Attack: 공격 시작 처리
	 * @param NewAttackTarget - 공격 대상 액터
	 */
	virtual void Attack_Implementation(AActor* NewAttackTarget) override;

	/**
	 * @brief AttackStart: 공격 시작 전, 토큰 예약 및 상태 변경 처리
	 * @param NewAttackTarget - 공격 대상 액터
	 * @param TokensNeeded - 필요한 공격 토큰 수
	 * @return 예약 성공 시 true, 실패 시 false
	 */
	virtual bool AttackStart_Implementation(AActor* NewAttackTarget, int32 TokensNeeded) override;

	/**
	 * @brief AttackEnd: 공격 종료 처리 (토큰 반환 및 상태 변경)
	 * @param NewAttackTarget - 공격 대상 액터
	 */
	virtual void AttackEnd_Implementation(AActor* NewAttackTarget) override;

	virtual void StoreAttackTokens_Implementation(AActor* AttackToTarget, int32 Amount) override;
	virtual float SetMoveSpeed_Implementation(EAIMovementSpeed NewMovementSpeed) override;
	virtual APatrolPath* GetPatrolPath_Implementation() override;

public:
	/** AIBehaviorsComponent: AI의 행동 로직을 제어하는 컴포넌트 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="AI Base")
	TObjectPtr<UAIBehaviorsComponent> AIBehaviorsComponent;

	// UAttackComponent: 공격 관련 로직을 담당하는 컴포넌트 (현재 주석 처리됨)
	// UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="AI Base")
	// TObjectPtr<UAttackComponent> AttackComponent;

	/** DamageSystemComponent: 체력 및 피해 처리 시스템 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="AI Base")
	TObjectPtr<UDamageSystemComponent> DamageSystemComponent;

	/** HealthWidgetComponent: 체력 UI 표시용 위젯 (블루프린트에서 설정 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base")
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;

	/** BehaviorTree: AI 행동 패턴을 정의하는 비헤이비어 트리 에셋 */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="AI Base")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** EnemyAIController: AI 캐릭터를 제어하는 컨트롤러 */
	UPROPERTY(BlueprintReadWrite, Category="AI Base")
	TObjectPtr<AEnemyAIController> EnemyAIController;

	/** 무기 착용 여부 (true: 무기 착용, false: 미착용) */
	UPROPERTY(BlueprintReadWrite, Category="AI Base")
	bool bIsWieldingWeapon;

	/** BlockChance: 피해 블록 시도 확률 (0~1, 기본값: 0.25) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base")
	float BlockChance = 0.25f;

	/** 팀 번호 (기본값: 2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base")
	int32 TeamNumber = 2;

	/** 현재 공격에 사용한 공격 토큰 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base")
	int32 TokensUsedInCurrentAttack;



	/** 공격 진행 상태 플래그 */
	UPROPERTY(BlueprintReadWrite, Category="AI Base")
	bool bAttacking;

	/** BlockMontage: 블록(방어) 시 재생할 애님 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base")
	TObjectPtr<UAnimMontage> BlockMontage;

	/** HitReactionMontage: 피해 시 재생할 히트 리액션 애님 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base")
	TObjectPtr<UAnimMontage> HitReactionMontage;

	/** StaggerMontage: 스태거(비틀거림) 시 재생할 애님 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base")
	TObjectPtr<UAnimMontage> StaggerMontage;

	/** PatrolRoute: AI의 순찰 경로를 지정하는 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base")
	TObjectPtr<APatrolPath> PatrolRoute;

	/** ReservedAttackTokens: 공격 예약 토큰을 관리하는 맵 (공격 대상, 토큰 수) */
	UPROPERTY(BlueprintReadOnly, Category="AI Base")
	TMap<AActor*, int> ReservedAttackTokens;

private:
	/** LastHitResponseMontage: 마지막으로 재생한 히트 리액션 몽타주 (HitResponse 관련) */
	UPROPERTY()
	TObjectPtr<UAnimMontage> LastHitResponseMontage;

	/** PendingDamageCauser: HitResponse 시 전달받은 DamageCauser를 임시 저장 (OnHitMontageEnded에서 사용) */
	UPROPERTY()
	TObjectPtr<AActor> PendingDamageCauser;

	/** HoldBlockTimerHandle: 블록 지속 시간을 관리하는 타이머 핸들 */
	FTimerHandle HoldBlockTimerHandle;

public:
	/** OnAttackEnd: 공격 종료 시 발생하는 델리게이트 (블루프린트 바인딩 가능) */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="AI Base|Delegate")
	FOnAttackEnd OnAttackEnd;

	/** OnWeaponEquipped: 무기 장착 시 발생하는 델리게이트 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="AI Base|Delegate")
	FOnWeaponEquipped OnWeaponEquipped;

	/** OnWeaponUnEquipped: 무기 해제 시 발생하는 델리게이트 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="AI Base|Delegate")
	FOnWeaponUnEquipped OnWeaponUnEquipped;

	/** OnBlockEnded: 블록 종료 시 발생하는 델리게이트 */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="AI Base|Delegate")
	FOnBlockEnded OnBlockEnded;
};
