#include "ShooterPro/Public/AI/ShooterAIBase.h"

#include "AI/Components/AIBehaviorsComponent.h"
#include "AI/Components/AICollisionComponent.h"
#include "AI/Utility/ShooterAIBluePrintFunctionLibrary.h"
#include "AI/ShooterAILog.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavAreas/NavArea_Obstacle.h"


AShooterAIBase::AShooterAIBase()
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
		GetMesh()->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Ignore);
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


	//AI Base Component 설정
	AIBehaviorsComponent = CreateDefaultSubobject<UAIBehaviorsComponent>(TEXT("AIBehaviorsComponent"));
	MeleeCollisionComponent = CreateDefaultSubobject<UAICollisionComponent>(TEXT("MeleeCollisionComponent"));

	//Dummy Curve를 설정해야합니다.
	// 예: CurveFloat 에셋 불러오기 (경로는 콘텐츠 브라우저에서 확인한 에셋 경로)
	// static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveAsset(TEXT("/Game/Curves/MySpeedCurve.MySpeedCurve"));
	// if (CurveAsset.Succeeded())
	// {
	// 	RotateCurve = CurveAsset.Object;
	// }
}


void AShooterAIBase::BeginPlay()
{
	Super::BeginPlay();

	if (!BehaviorTree)
	{
		SHOOTERAI_LOG_ERROR("BehaviorTree 가 nullptr 입니다.");
	}

	if (RotateCurve)
	{
		FOnTimelineFloat OnRotate;
		OnRotate.BindUFunction(this, FName("OnRotateUpdateAlpha"));
		RotateTimeline.AddInterpFloat(RotateCurve, OnRotate);

		RotateTimeline.SetTimelineLength(RotateTimelineLength);
		RotateTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
	}
	else
	{
		SHOOTERAI_LOG_ERROR("RotateCurve 가 nullptr 입니다.");
	}
}

float AShooterAIBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth -= ActualDamage;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		if (const FPointDamageEvent* PointEvent = static_cast<const FPointDamageEvent*>(&DamageEvent))
		{
			EHitDirection HitDirection = UShooterAIBluePrintFunctionLibrary::GetHitDirection(PointEvent->ShotDirection, this);

			if (CurrentHealth <= 0)
			{
				AIBehaviorsComponent->DeadRagdoll(PointEvent->ShotDirection, PointEvent->HitInfo.BoneName, 1000.0f, HitDirection);
			}
			else
			{
				AIBehaviorsComponent->Hitted(HitDirection);
			}
		}
	}

	return ActualDamage;
}

void AShooterAIBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RotateTimeline.TickTimeline(DeltaSeconds);
}

void AShooterAIBase::StartRotateToPlayer(float NewInterpSpeed)
{
	RotateInterpSpeed = NewInterpSpeed;
	RotateTimeline.PlayFromStart();
}

void AShooterAIBase::StopRotateToPlayer()
{
	RotateTimeline.Stop();
}

void AShooterAIBase::OnRotateUpdateAlpha(float Alpha)
{
	AActor* AITarget = AIBehaviorsComponent->GetTargetActor();
	if (IsValid(AITarget))
	{
		FVector SelfLocation = GetActorLocation();
		FVector TargetLocation = AITarget->GetActorLocation();

		float TargetYaw = UKismetMathLibrary::FindLookAtRotation(SelfLocation, TargetLocation).Yaw;
		FRotator TargetRotator = FRotator(0, TargetYaw, 0);

		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotator, GetWorld()->GetDeltaSeconds(), RotateInterpSpeed));
	}
}
