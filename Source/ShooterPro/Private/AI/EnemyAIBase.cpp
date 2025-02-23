#include "ShooterPro/Public/AI/EnemyAIBase.h"

#include "BrainComponent.h"

#include "AI/EnemyAIController.h"
#include "AI/Components/AIBehaviorsComponent.h"

#include "AI/EnemyAILog.h"

#include "AI/Components/DamageSystemComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

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

	// AI 행동 컴포넌트 생성 (AI의 행동 로직을 제어)
	AIBehaviorsComponent = CreateDefaultSubobject<UAIBehaviorsComponent>(TEXT("AIBehaviorsComponent"));
	// 피해 처리 컴포넌트 생성 (체력, 피해 계산 등)
	DamageSystemComponent = CreateDefaultSubobject<UDamageSystemComponent>(TEXT("DamageSystemComponent"));
	// 공격 컴포넌트는 현재 주석 처리됨
	// AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));

	// 체력 UI 표시용 위젯 생성 및 SkeletalMesh에 부착
	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidgetComponent"));
	HealthWidgetComponent->SetupAttachment(GetMesh());
}

/**
 * @brief BeginPlay: 게임 시작 시 호출되는 초기화 함수
 */
void AEnemyAIBase::BeginPlay()
{
	Super::BeginPlay();

	// AI 컨트롤러 가져오기 (AI 캐릭터를 제어할 컨트롤러)
	EnemyAIController = Cast<AEnemyAIController>(UAIBlueprintHelperLibrary::GetAIController(this));
	if (EnemyAIController)
	{
		// 체력 초기화 (예: 100)
		DamageSystemComponent->Health = 100.0f;
		DamageSystemComponent->MaxHealth = 100.0f;

		// 피해 및 사망, 블록 이벤트에 대한 델리게이트 바인딩
		DamageSystemComponent->OnDeath.AddDynamic(this, &AEnemyAIBase::Die);
		DamageSystemComponent->OnDamageResponse.AddDynamic(this, &AEnemyAIBase::HitResponse);
		DamageSystemComponent->OnBlocked.AddDynamic(this, &AEnemyAIBase::OnBlocked);
	}

	// 블루프린트에서 Health Widget 설정 여부 확인 (필요 시 추가 처리)
}

void AEnemyAIBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AEnemyAIBase::Die()
{
	StopAnimMontage();
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	EnemyAIController->GetBrainComponent()->StopLogic("");
	EnemyAIController->SetStateAsDead();
	HealthWidgetComponent->SetVisibility(false, true);

	// 예약된 공격 토큰들을 반환 처리
	for (const TTuple<AActor*, int>& ReservedAttackToken : ReservedAttackTokens)
	{
		if (ReservedAttackToken.Key->Implements<UInterface_Damageable>())
		{
			IInterface_Damageable::Execute_ReturnAttackToken(ReservedAttackToken.Key, ReservedAttackToken.Value);
		}
	}
}

void AEnemyAIBase::HitResponse(EDamageResponse DamageResponse, AActor* DamageCauser)
{
	// 이동 정지 및 AI 상태를 '동결' 상태로 설정
	GetCharacterMovement()->StopMovementImmediately();
	EnemyAIController->SetStateAsFrozen();

	// 적절한 히트 리액션 몽타주 선택
	UAnimMontage* SelectHitMontage = nullptr;
	if (DamageResponse == EDamageResponse::None ||
		DamageResponse == EDamageResponse::HitReaction ||
		DamageResponse == EDamageResponse::Stun ||
		DamageResponse == EDamageResponse::KnockBack)
	{
		SelectHitMontage = HitReactionMontage;
	}
	else if (DamageResponse == EDamageResponse::Stagger)
	{
		SelectHitMontage = IsValid(StaggerMontage) ? StaggerMontage : HitReactionMontage;
	}

	// 몽타주 재생 및 종료 델리게이트 바인딩
	if (SelectHitMontage)
	{
		if (USkeletalMeshComponent* MeshComp = GetMesh())
		{
			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				// 선택된 몽타주 재생 (PlayRate: 1.0, 시작 위치: 0)
				float MontageLength = AnimInstance->Montage_Play(SelectHitMontage, 1.f);
				if (MontageLength > 0.f)
				{
					// 재생한 몽타주를 LastHitResponseMontage에 저장 (후속 비교용)
					LastHitResponseMontage = SelectHitMontage;

					// DamageCauser를 임시 저장하여 몽타주 종료 후 처리에 사용
					PendingDamageCauser = DamageCauser;

					// 몽타주 종료 시 호출될 델리게이트 바인딩
					FOnMontageEnded MontageEndedDelegate;
					MontageEndedDelegate.BindUObject(this, &AEnemyAIBase::OnHitMontageEnded);
					AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, SelectHitMontage);
				}
			}
		}
	}

	// (아래의 공격 상태 전환 로직은 OnHitMontageEnded에서 처리됨)
}

void AEnemyAIBase::OnBlocked(bool bCanBeParried, AActor* DamageCauser)
{
	// 기존 블록 타이머를 해제
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, HoldBlockTimerHandle);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			// 블록 몽타주 재생 (PlayRate: 1.0, 시작 위치: 0)
			float MontageLength = AnimInstance->Montage_Play(BlockMontage, 1.f);
			if (MontageLength > 0.f)
			{
				// 블록 몽타주 종료 시 호출될 델리게이트 바인딩 (OnHitMontageEnded 재사용)
				FOnMontageEnded MontageEndedDelegate;
				MontageEndedDelegate.BindUObject(this, &AEnemyAIBase::OnHitMontageEnded);
				AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, BlockMontage);
			}
		}
	}
}

void AEnemyAIBase::OnHitMontageEnded(UAnimMontage* AnimMontage, bool bInterrupted)
{
	// HitResponse 관련 몽타주 종료 처리
	if (AnimMontage == LastHitResponseMontage)
	{
		if (bInterrupted)
		{
			// 중단된 경우 로그 출력 (한글 로그)
			AI_ENEMY_SCREEN_LOG_WARNING(5.0f,"HitResponse -> 히트 몽타주가 중단되었습니다.");
		}
		else
		{
			// 정상 종료된 경우 로그 출력
			AI_ENEMY_SCREEN_LOG_WARNING(5.0f,"HitResponse -> 히트 몽타주가 정상 종료되었습니다.");
		}

		// PendingDamageCauser가 유효하면, 팀 번호를 비교하여 공격 상태 전환 수행
		if (PendingDamageCauser && PendingDamageCauser->Implements<UInterface_Damageable>())
		{
			int32 DamageCauserTeamNumber = IInterface_Damageable::Execute_GetTeamNumber(PendingDamageCauser);
			// 자신의 팀 번호와 다르면 공격 대상으로 설정
			AActor* AttackTarget = (TeamNumber != DamageCauserTeamNumber) ? PendingDamageCauser : nullptr;
			AIBehaviorsComponent->SetStateAsAttacking(AttackTarget, true);
		}

		// 사용한 임시 멤버 변수 초기화
		PendingDamageCauser = nullptr;
		LastHitResponseMontage = nullptr;
	}
	// OnBlocked 관련 몽타주 종료 처리
	else if (AnimMontage == BlockMontage)
	{
		if (bInterrupted)
		{
			AI_ENEMY_SCREEN_LOG_WARNING(5.0f,"OnBlocked -> 블록 몽타주가 중단되었습니다.");
		}
		else
		{
			AI_ENEMY_SCREEN_LOG_WARNING(5.0f,"OnBlocked -> 블록 몽타주가 정상 종료되었습니다.");
		}

		// 블록 종료 함수 호출
		EndBlock();
	}
}

void AEnemyAIBase::StartBlock()
{
	// 기존 타이머 해제
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, HoldBlockTimerHandle);
	GetCharacterMovement()->StopMovementImmediately();
	// 블록 상태 초기화
	DamageSystemComponent->bIsBlocking = false;
	GetCharacterMovement()->StopMovementImmediately();
	// EndBlock 함수를 2초 후 호출하는 타이머 설정
	HoldBlockTimerHandle = UKismetSystemLibrary::K2_SetTimer(this, "EndBlock", 2.0f, false);
}

void AEnemyAIBase::TryToBlock()
{
	if (FMath::FRand() <= BlockChance)
		StartBlock();
}

void AEnemyAIBase::EndBlock()
{
	DamageSystemComponent->bIsBlocking = false;

	if (OnBlockEnded.IsBound())
		OnBlockEnded.Broadcast();
}

/* ========================= Interface_Damageable 구현 ========================= */

void AEnemyAIBase::ReturnAttackToken_Implementation(int Amount)
{
	DamageSystemComponent->ReturnAttackToken(Amount);
}

void AEnemyAIBase::SetIsInterruptible_Implementation(bool bNewIsInterruptible)
{
	DamageSystemComponent->bIsInterruptible = bNewIsInterruptible;
}

void AEnemyAIBase::SetIsInvincible_Implementation(bool bNewIsInvincible)
{
	DamageSystemComponent->bIsInvincible = bNewIsInvincible;
}

int32 AEnemyAIBase::GetTeamNumber_Implementation()
{
	return TeamNumber;
}

bool AEnemyAIBase::ReserveAttackToken_Implementation(int Amount)
{
	return DamageSystemComponent->ReserveAttackToken(Amount);
}

bool AEnemyAIBase::IsAttacking_Implementation()
{
	return bAttacking;
}

bool AEnemyAIBase::IsDead_Implementation()
{
	return DamageSystemComponent->bIsDead;
}

bool AEnemyAIBase::TakeDamage_Implementation(FDamageInfo DamageInfo, AActor* DamageCauser)
{
	// 피해가 블록 가능하면 블록 시도
	if (DamageInfo.bCanBeBlocked)
		TryToBlock();

	return DamageSystemComponent->TakeDamage(DamageInfo, DamageCauser);
}

float AEnemyAIBase::Heal_Implementation(float Amount)
{
	return DamageSystemComponent->Heal(Amount);
}

float AEnemyAIBase::GetMaxHealth_Implementation()
{
	return DamageSystemComponent->MaxHealth;
}

float AEnemyAIBase::GetCurrentHealth_Implementation()
{
	return DamageSystemComponent->Health;
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

void AEnemyAIBase::Attack_Implementation(AActor* NewAttackTarget)
{
	bAttacking = true;
}

bool AEnemyAIBase::AttackStart_Implementation(AActor* NewAttackTarget, int32 TokensNeeded)
{
	// 공격 대상이 Damageable 또는 EnemyAI 인터페이스를 구현하는지 확인
	if (!NewAttackTarget->Implements<UInterface_Damageable>() && !NewAttackTarget->Implements<UInterface_EnemyAI>())
		return false;

	// 공격 대상의 예약 토큰 예약 시도 (실패 시 false 반환)
	if (!IInterface_Damageable::Execute_ReserveAttackToken(NewAttackTarget, TokensNeeded))
		return false;

	// 공격 토큰 저장 및 현재 공격에 사용한 토큰 수 기록
	IInterface_EnemyAI::Execute_StoreAttackTokens(this, NewAttackTarget, TokensNeeded);
	TokensUsedInCurrentAttack = TokensNeeded;
	return true;
}

void AEnemyAIBase::AttackEnd_Implementation(AActor* NewAttackTarget)
{
	// 공격 대상이 Damageable 인터페이스를 구현하는 경우
	if (!NewAttackTarget->Implements<UInterface_Damageable>())
		return;

	// 공격 토큰 반환 및 저장, 공격 상태 플래그 해제
	IInterface_Damageable::Execute_ReturnAttackToken(NewAttackTarget, TokensUsedInCurrentAttack);
	IInterface_EnemyAI::Execute_StoreAttackTokens(this, NewAttackTarget, -TokensUsedInCurrentAttack);
	bAttacking = false;

	// OnAttackEnd 델리게이트 방송 (블루프린트 이벤트)
	if (OnAttackEnd.IsBound())
		OnAttackEnd.Broadcast();
}

void AEnemyAIBase::StoreAttackTokens_Implementation(AActor* AttackToTarget, int32 Amount)
{
	int32 NewAmount = ReservedAttackTokens.Contains(AttackToTarget) ? ReservedAttackTokens[AttackToTarget] + Amount : Amount;
	ReservedAttackTokens.Add(AttackToTarget, NewAmount);
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
