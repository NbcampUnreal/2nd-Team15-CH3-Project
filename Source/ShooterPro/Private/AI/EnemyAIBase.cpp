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
}

void AEnemyAIBase::BeginPlay()
{
	Super::BeginPlay();

	EnemyAIController = Cast<AEnemyAIController>(UAIBlueprintHelperLibrary::GetAIController(this));

	if (GscCoreComponent)
		GscCoreComponent->OnAbilityEnded.AddDynamic(this, &AEnemyAIBase::OnAbilityEndedCallback);

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
