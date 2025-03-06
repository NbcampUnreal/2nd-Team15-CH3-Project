#include "ShooterPro/Public/AI/EnemyAIBase.h"


#include "MotionWarpingComponent.h"
#include "ProGmaeplayTag.h"
#include "AI/AIGameplayTags.h"
#include "AI/EnemyAIController.h"
#include "AI/Components/AIBehaviorsComponent.h"

#include "AI/EnemyAILog.h"

#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/GSCCoreComponent.h"
#include "Components/WidgetComponent.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"


AEnemyAIBase::AEnemyAIBase()
{
	// 매 프레임 Tick 함수 호출 활성화
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidgetComponent"));
	HealthWidgetComponent->SetupAttachment(GetRootComponent());

	AIBehaviorsComponent = CreateDefaultSubobject<UAIBehaviorsComponent>(TEXT("AIBehaviorsComponent"));
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	GscCoreComponent = CreateDefaultSubobject<UGSCCoreComponent>(TEXT("GscCoreComponent"));
	bIsAlive = true;
}

// 바인딩된 함수들을 액터 사망시 실행시킬 함수 - 김민재 추가
void AEnemyAIBase::EnemyOnKilled()
{
	// 1) 사망 관련 로직 (예: 델리게이트 브로드캐스트)
	OnKilledActor.Broadcast(this);

	// 2) 실제 파괴 대신 풀링용 비활성화 처리
	DeactivateForPooling();
}

void AEnemyAIBase::DeactivateForPooling()
{
	// 1) 움직임 정지 및 충돌 비활성화
	GetCharacterMovement()->StopMovementImmediately();
	SetActorEnableCollision(false);

	// 2) 눈에 안 보이게 숨기기 (메시나 위젯도 포함해서)
	SetActorHiddenInGame(true);

	bIsAlive = false;

	// 3) AIController 논리 중지 (원한다면)
	if (EnemyAIController)
	{
		EnemyAIController->StopMovement();
		// BehaviorTree나 Blackboard 초기화도 가능
	}

	// 여기서는 Destroy()하지 않고, 풀에서 "죽은 상태"로 대기
}

void AEnemyAIBase::OnPooledRespawn()
{
	// 1) 숨김 해제 & 충돌 다시 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	bIsAlive = true;

	// 2) 체력/스태미나 등의 스탯을 최대치로 복원
	if (GscCoreComponent)
	{
		// 예시: GscCoreComponent가 Health Attribute를 가지고 있다면
		GscCoreComponent->RestoreToMaxValues();
		// 또는 직접 ASC에 접근해서 Health를 세팅할 수도 있습니다.
	}

	// 3) AI 로직 다시 시작
	if (EnemyAIController)
	{
		// Behavior Tree 재시작, 블랙보드 초기화 등
		if (BehaviorTree)
		{
			EnemyAIController->RunBehaviorTree(BehaviorTree);
		}
		// 필요 시 AI 상태값도 초기화(Idle 상태 등)
	}

	// 5) 기타 필요한 변수들 초기화 (AIBehaviorsComponent 등)
	if (AIBehaviorsComponent)
	{
		EnemyAIController->UpdateBlackboard_State(AIGameplayTags::AIState_Idle);
		EnemyAIController->UpdateBlackboard_AttackRadius(AIBehaviorsComponent->GetAttackRadius());
		EnemyAIController->UpdateBlackboard_DefendRadius(AIBehaviorsComponent->GetDefendRadius());
		EnemyAIController->UpdateBlackboard_StartLocation(GetActorLocation());
		EnemyAIController->UpdateBlackboard_MaxRandRadius(AIBehaviorsComponent->GetMaxRandRadius());
	}
}

void AEnemyAIBase::BeginPlay()
{
	Super::BeginPlay();

	EnemyAIController = Cast<AEnemyAIController>(UAIBlueprintHelperLibrary::GetAIController(this));

	if (GscCoreComponent)
		GscCoreComponent->OnAbilityEnded.AddDynamic(this, &AEnemyAIBase::OnAbilityEndedCallback);

	// 블루프린트에서 Health Widget 설정 여부 확인 (필요 시 추가 처리)
}

void AEnemyAIBase::OnAbilityEndedCallback(const UGameplayAbility* EndedAbility)
{
	if (!EndedAbility)
		return;

	// 만약 Subsystem이 이미 죽었거나 생성 안 되었으면 그냥 반환
	if (!UGameplayMessageSubsystem::HasInstance(EndedAbility))
	{
		return;
	}

	// 1) 어빌리티의 태그들
	FGameplayTagContainer AbilityTags = EndedAbility->GetAssetTags();
	FGameplayTag BroadcastTag = ProGameplayTags::Ability;
	if (!AbilityTags.IsEmpty())
	{
		BroadcastTag = *AbilityTags.CreateConstIterator();
	}

	// 2) 보낼 Payload 구성 (새로운 구조체 사용)
	FEnemyAbilityEndedPayload Payload;
	Payload.EndedAbilityName = EndedAbility->GetName();
	Payload.EndedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	Payload.AbilityOwner = this; // AIBase 자신 (어빌리티 소유자)

	// 대표 태그
	Payload.EndedAbilityTag = BroadcastTag;

	// 3) 메시지 전송
	UGameplayMessageSubsystem& MsgSubsystem = UGameplayMessageSubsystem::Get(EndedAbility);
	MsgSubsystem.BroadcastMessage<FEnemyAbilityEndedPayload>(BroadcastTag, Payload);
}

void AEnemyAIBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

bool AEnemyAIBase::IsDead_Implementation()
{
	return !bIsAlive;
}


/* ========================= Interface_EnemyAI 구현 ========================= */
void AEnemyAIBase::JumpToDestination_Implementation(FVector NewDestination)
{
	// 시작 위치와 도착 위치 계산 (도착 위치는 높이 오프셋 추가)
	FVector StartLocation = GetActorLocation();
	FVector EndLocation = NewDestination + FVector(0.f, 0.f, 250.f);
	FVector LaunchVelocity;
	// 점프(발사) 속도 계산 (포물선 궤적)
	UGameplayStatics::SuggestProjectileVelocity_CustomArc(this, LaunchVelocity, StartLocation, EndLocation);
	LaunchCharacter(LaunchVelocity, true, true);
}

float AEnemyAIBase::SetMoveSpeed_Implementation(EAIMovementSpeed NewMovementSpeed)
{
	if (!AIBehaviorsComponent)
		return 0.f;

	switch (NewMovementSpeed)
	{
	case EAIMovementSpeed::Idle:
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		break;
	case EAIMovementSpeed::Walking:
		GetCharacterMovement()->MaxWalkSpeed = AIBehaviorsComponent->WalkSpeed;
		break;
	case EAIMovementSpeed::Jogging:
		GetCharacterMovement()->MaxWalkSpeed = AIBehaviorsComponent->JogSpeed;
		break;
	case EAIMovementSpeed::Sprinting:
		GetCharacterMovement()->MaxWalkSpeed = AIBehaviorsComponent->SprintingSpeed;
		break;
	}

	return GetCharacterMovement()->MaxWalkSpeed;
}

APatrolPath* AEnemyAIBase::GetPatrolPath_Implementation()
{
	return PatrolRoute;
}
