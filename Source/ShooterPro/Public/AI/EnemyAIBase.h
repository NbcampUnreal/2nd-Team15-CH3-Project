#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Interfaces/Interface_EnemyAI.h"
#include "ModularGameplayActors/GSCModularCharacter.h"
#include "EnemyAIBase.generated.h"


class UGSCCoreComponent;
class UMotionWarpingComponent;
class UWidgetComponent;
class AEnemyAIController;
class UAICollisionComponent;
class UAIBehaviorsComponent;
class UBehaviorTree;
class APatrolPath;


/**
 * AEnemyAIBase 클래스
 * 
 * AI 적 캐릭터의 기본 동작을 담당하며, 피해 처리 및 AI 관련 인터페이스를 구현한다.
 * IInterface_Damageable, IInterface_EnemyAI 인터페이스를 상속받아 다양한 액션(공격, 이동, 블록 등)을 처리함.
 */
UCLASS()
class SHOOTERPRO_API AEnemyAIBase : public AGSCModularCharacter, public IInterface_EnemyAI
{
	GENERATED_BODY()

public:
	/** 생성자 */
	AEnemyAIBase();

protected:
	/** 게임 시작 시 또는 스폰 후 최초 호출 */
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnAbilityEndedCallback(const UGameplayAbility* EndedAbility);

public:
	/** 매 프레임마다 호출되는 함수 */
	virtual void Tick(float DeltaSeconds) override;

	virtual bool IsDead_Implementation() override;


	//=============================================================================
	// Interface_EnemyAI 구현 (이동, 공격, 순찰 등)
	//=============================================================================
public:
	virtual void JumpToDestination_Implementation(FVector NewDestination) override;

	virtual float SetMoveSpeed_Implementation(EAIMovementSpeed NewMovementSpeed) override;

	virtual APatrolPath* GetPatrolPath_Implementation() override;

public:
	UPROPERTY( BlueprintReadWrite, Category="AI Base")
	bool bIsAlive = true;
	
	/** AIBehaviorsComponent: AI의 행동 로직을 제어하는 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Base|Component")
	TObjectPtr<UAIBehaviorsComponent> AIBehaviorsComponent;

	/** HealthWidgetComponent: 체력 UI 표시용 위젯 (블루프린트에서 설정 가능) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Base|Component")
	TObjectPtr<UWidgetComponent> HealthWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Base|Component")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI Base|Component")
	TObjectPtr<UGSCCoreComponent> GscCoreComponent;
	
	/** EnemyAIController: AI 캐릭터를 제어하는 컨트롤러 */
	UPROPERTY(BlueprintReadWrite, Category="AI Base|Rference")
	TObjectPtr<AEnemyAIController> EnemyAIController;

	/** BehaviorTree: AI 행동 패턴을 정의하는 비헤이비어 트리 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Base|Config")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base|Config")
	FGameplayTag EnemyIdentifier;

	/** 팀 번호 (기본값: 2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base|Config")
	int32 TeamNumber = 2;

	/** PatrolRoute: AI의 순찰 경로를 지정하는 에셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Base|Config")
	TObjectPtr<APatrolPath> PatrolRoute;

	
};
