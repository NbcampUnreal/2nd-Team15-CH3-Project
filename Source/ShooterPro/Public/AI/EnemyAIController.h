#pragma once

#include "CoreMinimal.h"
#include "AIDectionInfoTypes.h"
#include "EnemyAITypes.h"
#include "DetourCrowdAIController.h"
#include "GameplayTagContainer.h"
#include "EnemyAIController.generated.h"


// Forward Declarations
class UProAIBehaviorsComponent;
class AEnemyAIBase;
class UPerceptionManager;

struct FAIStimulus;


/**
 * @brief AEnemyAIController 클래스
 * 
 * 적 AI의 다양한 상태(공격, 조사, 탐색, 죽음 등)를 제어하고,
 * AI Perception을 통해 주변 환경을 감지하여 상태를 전환합니다.
 */
UCLASS()
class SHOOTERPRO_API AEnemyAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()

	//=============================================================================
	// 초기화 및 라이프사이클 함수
	//=============================================================================
public:
	/** 기본 생성자 */
	AEnemyAIController();

protected:
	/** 게임 시작 시 호출되는 함수 */
	virtual void BeginPlay() override;

	/** Pawn이 AIController에 소유될 때 호출되는 함수 */
	virtual void OnPossess(APawn* InPawn) override;

	/** Pawn이 AIController로부터 해제될 때 호출되는 함수 */
	virtual void OnUnPossess() override;

public:
	/** 매 틱마다 호출되는 함수 */
	virtual void Tick(float DeltaTime) override;


	//=============================================================================
	// 감지 및 이벤트 처리 함수
	//=============================================================================
public:
	/** 감지 정보 업데이트 */
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	void UpdatePerception(float DeltaTime);

	/** 현재 소유한 Pawn과 지정된 액터가 같은 팀인지 검사 */
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	bool OnSameTeam(AActor* Actor);

	/** 감지 정보 관리 객체 반환 */
	UFUNCTION(BlueprintPure, Category="Enemy AI Controller")
	UPerceptionManager* GetDetectionInfoManager() const { return DetectionInfoManager; }


	/** 감각(Sight, Hearing, Damage) 기반 액터 감지 여부 확인 */
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller")
	bool CanPerceiveActor(AActor* Actor, EAISense SenseType, FAIStimulus& OutAIStimulus);

protected:
	/** 감지된 액터 목록 업데이트 이벤트 처리 */
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	/** 감지된 액터 목록에서 잊혀진 액터 처리 */
	UFUNCTION()
	void OnTargetPerceptionForgotten(AActor* ForgottenActor);


	//=============================================================================
	// Blackboard 관련 함수 (상태 및 데이터 업데이트)
	//=============================================================================
public:
	/** 블랙보드에 AI 상태(GameplayTag)를 설정한다 */
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void UpdateBlackboard_State(EAIState NewState);

	/** 블랙보드에서 현재 AI 상태(GameplayTag)를 가져온다 */
	UFUNCTION(BlueprintPure, Category="Enemy AI Controller|Blackboard")
	EAIState GetCurrentState() const;

	UFUNCTION(BlueprintPure, Category="Enemy AI Controller|Blackboard")
	EAIState GetPreviousState() const;

	/** Blackboard의 공격 반경(Attack Radius) 업데이트 */
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void UpdateBlackboard_AttackRadius(float NewAttackRadius);

	/** Blackboard의 방어 반경(Defend Radius) 업데이트 */
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void UpdateBlackboard_DefendRadius(float NewDefendRadius);

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void UpdateBlackboard_StartLocation(FVector NewDefendRadius);

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void UpdateBlackboard_MaxRandRadius(float NewDefendRadius);

	/** Blackboard의 관심 지점(Point of Interest) 업데이트 */
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void UpdateBlackboard_PointOfInterest(FVector NewPointOfInterest);

	/** Blackboard의 공격 대상 업데이트 */
	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void UpdateBlackboard_AttackTarget(UObject* NewAttackTarget);

	UFUNCTION(BlueprintCallable, Category="Enemy AI Controller|Blackboard")
	void UpdateBlackboard_AttackTarget_ClearValue();

	//=============================================================================
	// 멤버 변수 (프로퍼티)
	//=============================================================================
public:
	/** AI Perception 컴포넌트 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category="Enemy AI Controller")
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	/** 소유한 적 AI 캐릭터 (AEnemyAIBase) */
	UPROPERTY(BlueprintReadWrite, Category="Enemy AI Controller|Reference")
	TObjectPtr<AEnemyAIBase> PossessedAI;

	/** AI 행동 제어 컴포넌트 (AIBehaviorsComponent) */
	UPROPERTY(BlueprintReadWrite, Category="Enemy AI Controller|Reference")
	TObjectPtr<UProAIBehaviorsComponent> AIBehaviorComponent;

protected:
	/** 감지 정보 관리 객체 */
	UPROPERTY(BlueprintReadWrite, Category="Detection")
	UPerceptionManager* DetectionInfoManager;
};
