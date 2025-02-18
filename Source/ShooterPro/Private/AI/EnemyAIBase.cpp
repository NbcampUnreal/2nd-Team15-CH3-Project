#include "ShooterPro/Public/AI/EnemyAIBase.h"

#include "AI/EnemyAIController.h"
#include "AI/Components/AIBehaviorsComponent.h"
#include "AI/Components/AICollisionComponent.h"
#include "AI/Utility/EnemyAIBluePrintFunctionLibrary.h"
#include "AI/EnemyAILog.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavAreas/NavArea_Obstacle.h"


AEnemyAIBase::AEnemyAIBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	//CapsuleComponent 설정
	{
		// Capsule 설정: CapsuleRadius, CapsuleHalfHeight
		GetCapsuleComponent()->InitCapsuleSize(35.0f, 90.0f);

		// 충돌 프로파일 설정
		GetCapsuleComponent()->SetCollisionProfileName(TEXT("Custom"));
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		// BodyInstance의 ObjectType을 Pawn으로 설정
		GetCapsuleComponent()->BodyInstance.SetObjectType(ECollisionChannel::ECC_Pawn);

		// 특정 채널에 대한 반응 설정
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);

		// Navigation 관련 설정: AreaClassOverride를 사용합니다.
		GetCapsuleComponent()->SetAreaClassOverride(UNavArea_Obstacle::StaticClass());
		GetCapsuleComponent()->bDynamicObstacle = true;

		// 캐릭터가 이 캡슐 위에서 step-up 하지 못하도록 설정
		GetCapsuleComponent()->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	}

	//SkeletalMeshComponent 설정
	{
		// 충돌 설정
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetMesh()->SetCollisionProfileName(TEXT("Custom"));

		// BodyInstance의 ObjectType을 PhysicsBody로 설정
		GetMesh()->BodyInstance.SetObjectType(ECollisionChannel::ECC_PhysicsBody);

		// Pawn, Vehicle 채널 무시, Camera 채널은 Overlap
		GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		GetMesh()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
		GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Overlap);

		// 상대 위치 및 회전 설정
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -92.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, 270.f, 0.f));
	}

	//CharacterMovement 설정
	{
		GetCharacterMovement()->GravityScale = 1.75f;
		GetCharacterMovement()->MaxAcceleration = 1500.0f;
		GetCharacterMovement()->BrakingFriction = 1.0f;
		GetCharacterMovement()->bUseSeparateBrakingFriction = true;

		GetCharacterMovement()->MaxWalkSpeed = 500.0f;
		GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

		GetCharacterMovement()->AirControl = 0.35f;
		GetCharacterMovement()->JumpZVelocity = 700.0f;

		GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
		GetCharacterMovement()->bUseControllerDesiredRotation = true;

		GetCharacterMovement()->bUseRVOAvoidance = true;
		GetCharacterMovement()->AvoidanceConsiderationRadius = 250.0f;

		// 네비 에이전트 반경/높이
		GetCharacterMovement()->NavAgentProps.AgentRadius = 42.0f;
		GetCharacterMovement()->NavAgentProps.AgentHeight = 192.0f;
	}

	AIBehaviorsComponent = CreateDefaultSubobject<UAIBehaviorsComponent>(TEXT("AIBehaviorsComponent"));
}


void AEnemyAIBase::BeginPlay()
{
	Super::BeginPlay();

	AEnemyAIController* EnemyAIController = Cast<AEnemyAIController>(UAIBlueprintHelperLibrary::GetAIController(this));
}

void AEnemyAIBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}
