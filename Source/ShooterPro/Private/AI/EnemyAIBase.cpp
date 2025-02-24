#include "ShooterPro/Public/AI/EnemyAIBase.h"


#include "MotionWarpingComponent.h"
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

	// CapsuleComponent, SkeletalMeshComponent, CharacterMovementComponent 등
	// 기본 컴포넌트 설정은 주석 처리되어 있으므로, 필요 시 해당 주석을 해제하여 사용.
	// (설정 내용: 캡슐 크기, 충돌 설정, 네비게이션 관련 설정 등)

	// 체력 UI 표시용 위젯 생성 및 SkeletalMesh에 부착
	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidgetComponent"));
	HealthWidgetComponent->SetupAttachment(GetRootComponent());


	AIBehaviorsComponent = CreateDefaultSubobject<UAIBehaviorsComponent>(TEXT("AIBehaviorsComponent"));
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	GscCoreComponent = CreateDefaultSubobject<UGSCCoreComponent>(TEXT("GscCoreComponent"));
}

void AEnemyAIBase::BeginPlay()
{
	Super::BeginPlay();

	// AI 컨트롤러 가져오기 (AI 캐릭터를 제어할 컨트롤러)
	EnemyAIController = Cast<AEnemyAIController>(UAIBlueprintHelperLibrary::GetAIController(this));
	if (EnemyAIController)
	{
	}
	// 블루프린트에서 Health Widget 설정 여부 확인 (필요 시 추가 처리)
}

void AEnemyAIBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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
