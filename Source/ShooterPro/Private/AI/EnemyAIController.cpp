#include "ShooterPro/Public/AI/EnemyAIController.h"


#include "Kismet/KismetSystemLibrary.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

#include "Runtime/AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "Runtime/AIModule/Classes/Perception/AIPerceptionComponent.h"

#include "ShooterPro/Public/AI/EnemyAIBase.h"
#include "ShooterPro/Public/AI/EnemyAILog.h"

#include "AI/Components/AIBehaviorsComponent.h"
#include "AI/Interfaces/Interface_Damagable.h"
#include "AI/Utility/EnemyAIBluePrintFunctionLibrary.h"


AEnemyAIController::AEnemyAIController()
{
	// Tick 설정: 매 프레임 Tick을 수행할 수 있도록 합니다.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// AI Perception 컴포넌트를 생성합니다.
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// -- Sight Config --
	if (UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISenseConfig_Sight")))
	{
		SightConfig->SightRadius = 1500.f;
		SightConfig->LoseSightRadius = 2000.f;
		SightConfig->PeripheralVisionAngleDegrees = 60.f;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->SetMaxAge(20.f);
		AIPerception->ConfigureSense(*SightConfig);
	}

	// -- Hearing Config --
	if (UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("AISenseConfig_Hearing")))
	{
		HearingConfig->HearingRange = 500.f;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->SetMaxAge(3.f);
		AIPerception->ConfigureSense(*HearingConfig);
	}

	// -- Damage Config --
	if (UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("AISenseConfig_Damage")))
	{
		DamageConfig->SetMaxAge(5.f);
		AIPerception->ConfigureSense(*DamageConfig);
	}

	// DominantSense 설정 (여기서는 Sight)
	AIPerception->SetDominantSense(UAISense_Sight::StaticClass());
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();

	AIPerception->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// AShooterAIBase 클래스 캐스팅 시도
	AIPossessed = Cast<AEnemyAIBase>(InPawn);
	if (!AIPossessed)
	{
		// InPawn을 AShooterAIBase로 캐스팅할 수 없을 때 로그
		AI_ENEMY_SCREEN_LOG_ERROR("InPawn을 AShooterAIBase로 캐스팅할 수 없습니다.");
		return;
	}

	// AI 캐릭터에 지정된 Behavior Tree가 유효한지 확인 후 실행
	if (IsValid(AIPossessed->BehaviorTree))
	{
		RunBehaviorTree(AIPossessed->BehaviorTree);
		BlackBoardUpdate_State(EAIState::Passive);

		if (IsValid(AIPossessed->AIBehaviorsComponent))
		{
			PossessedBehaviorsComp = AIPossessed->AIBehaviorsComponent;

			BlackBoardUpdate_AttackRadius(PossessedBehaviorsComp->GetAttackRadius());
			BlackBoardUpdate_DefendRadius(PossessedBehaviorsComp->GetDefendRadius());

			CheckForgottenActorsTimer = UKismetSystemLibrary::K2_SetTimer(this, "CheckIfForgottenSeenActor", 0.5f, true);
		}
	}
	else
	{
		// Behavior Tree가 유효하지 않을 때 로그
		AI_ENEMY_SCREEN_LOG_ERROR("Behavior Tree가 유효하지 않습니다.");
	}
}

void AEnemyAIController::OnUnPossess()
{
	Super::OnUnPossess();
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, CheckForgottenActorsTimer);
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyAIController::SetStateAsAttacking(AActor* NewAttackTarget, bool bUseLastKnownAttackTarget)
{
	AActor* BestAttackTarget = (IsValid(AttackTarget) && bUseLastKnownAttackTarget) ? AttackTarget : NewAttackTarget;

	if (!IsValid(BestAttackTarget))
	{
		BlackBoardUpdate_State(EAIState::Passive);
		return;
	}

	if (BestAttackTarget->Implements<UInterface_Damagable>())
	{
		if (IInterface_Damagable::Execute_IsDead(BestAttackTarget))
		{
			BlackBoardUpdate_State(EAIState::Passive);
			return;
		}
	}

	BlackBoardUpdate_AttackTarget(BestAttackTarget);
	BlackBoardUpdate_State(EAIState::Attacking);
	AttackTarget = BestAttackTarget;
}

void AEnemyAIController::SetStateAsInvestigating(const FVector& Location)
{
	BlackBoardUpdate_State(EAIState::Investigating);
	BlackBoardUpdate_PointOfInterest(Location);
}

void AEnemyAIController::SetStateAsDead()
{
	BlackBoardUpdate_State(EAIState::Dead);
}

void AEnemyAIController::SetStateAsFrozen()
{
	BlackBoardUpdate_State(EAIState::Frozen);
}

void AEnemyAIController::SeekAttackTarget()
{
	BlackBoardUpdate_State(EAIState::Seeking);
	BlackBoardUpdate_PointOfInterest(AttackTarget->GetActorLocation());
	UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, SeekAttackTargetTimer);
}

void AEnemyAIController::CheckIfForgottenSeenActor()
{
	TArray<AActor*> SightPerceivedActors;
	PerceptionComponent->GetKnownPerceivedActors(UAISense_Sight::StaticClass(), SightPerceivedActors);

	if (KnownSeenActors.Num() != SightPerceivedActors.Num())
	{
		for (AActor* KnownSeenActor : KnownSeenActors)
		{
			int32 FindIndex = SightPerceivedActors.Find(KnownSeenActor);
			if (FindIndex == INDEX_NONE)
			{
				HandleForgotActor(KnownSeenActor);
			}
		}
	}
}

void AEnemyAIController::HandleForgotActor(AActor* Actor)
{
	KnownSeenActors.Remove(Actor);

	if (Actor == AttackTarget)
		BlackBoardUpdate_State(EAIState::Passive);
}

void AEnemyAIController::HandleSensedSight(AActor* Actor)
{
	KnownSeenActors.AddUnique(Actor);
	EAIState CurrentState = GetCurrentState();

	if (OnSameTeam(Actor))
		return;

	if (CurrentState == EAIState::Passive || CurrentState == EAIState::Investigating || CurrentState == EAIState::Seeking)
	{
		SetStateAsAttacking(Actor, false);
	}
	else if (CurrentState == EAIState::Attacking)
	{
		if (Actor == AttackTarget)
		{
			UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, SeekAttackTargetTimer);
		}
	}
}

void AEnemyAIController::HandleLostSight(AActor* Actor)
{
	if (Actor != AttackTarget)
		return;

	EAIState CurrentState = GetCurrentState();
	if (CurrentState == EAIState::Attacking || CurrentState == EAIState::Frozen || CurrentState == EAIState::Investigating)
	{
		UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, SeekAttackTargetTimer);
		SeekAttackTargetTimer = UKismetSystemLibrary::K2_SetTimer(this, "SeekAttackTarget", 1.f, false);
	}
}

void AEnemyAIController::HandleSensedSound(const FVector& Location)
{
	EAIState CurrentState = GetCurrentState();
	if (CurrentState == EAIState::Passive || CurrentState == EAIState::Investigating || CurrentState == EAIState::Seeking)
	{
		SetStateAsInvestigating(Location);
	}
}

void AEnemyAIController::HandleSensedDamage(AActor* Actor)
{
	if (OnSameTeam(Actor))
		return;

	EAIState CurrentState = GetCurrentState();

	if (CurrentState == EAIState::Passive || CurrentState == EAIState::Investigating || CurrentState == EAIState::Seeking)
	{
		SetStateAsAttacking(Actor, false);
	}
}

bool AEnemyAIController::CanSenseActor(AActor* Actor, EAISense SenseType, FAIStimulus& OutAIStimulus)
{
	FActorPerceptionBlueprintInfo Info;
	AIPerception->GetActorsPerception(Actor, Info);

	for (FAIStimulus LastSensedStimulus : Info.LastSensedStimuli)
	{
		TSubclassOf<UAISense> SenseClassForStimulus = UAIPerceptionSystem::GetSenseClassForStimulus(this, LastSensedStimulus);

		bool bFindScene = false;
		if (SenseType == EAISense::Sight) bFindScene = SenseClassForStimulus == UAISense_Sight::StaticClass();
		else if (SenseType == EAISense::Hearing) bFindScene = SenseClassForStimulus == UAISense_Hearing::StaticClass();
		else if (SenseType == EAISense::Damage) bFindScene = SenseClassForStimulus == UAISense_Damage::StaticClass();
		else bFindScene = SenseClassForStimulus == nullptr;

		if (bFindScene)
		{
			OutAIStimulus = LastSensedStimulus;
			return LastSensedStimulus.WasSuccessfullySensed();
		}
	}

	return false;
}

bool AEnemyAIController::OnSameTeam(AActor* Actor)
{
	if (!(Actor->Implements<UInterface_Damagable>() && GetPawn()->Implements<UInterface_Damagable>()))
	{
		AI_ENEMY_SCREEN_LOG_ERROR("Actor와 Pawn이 Interface_Damagable을 구현하지 않았습니다.");
		return true;
	}

	return IInterface_Damagable::Execute_GetTeamNumber(Actor) == IInterface_Damagable::Execute_GetTeamNumber(GetPawn());
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& Actors)
{
	for (AActor* Actor : Actors)
	{
		FAIStimulus AIStimulus;
		if (CanSenseActor(Actor, EAISense::Sight, AIStimulus))
		{
			HandleSensedSight(Actor);
		}
		else
		{
			HandleLostSight(Actor);
		}

		if (CanSenseActor(Actor, EAISense::Hearing, AIStimulus))
		{
			HandleSensedSound(AIStimulus.StimulusLocation);
		}

		if (CanSenseActor(Actor, EAISense::Damage, AIStimulus))
		{
			HandleSensedDamage(Actor);
		}
	}
}

EAIState AEnemyAIController::GetCurrentState()
{
	return static_cast<EAIState>(GetBlackboardComponent()->GetValueAsEnum(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_State()));
}

void AEnemyAIController::BlackBoardUpdate_State(EAIState NewState)
{
	GetBlackboardComponent()->SetValueAsEnum(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_State(), static_cast<uint8>(NewState));
}

void AEnemyAIController::BlackBoardUpdate_AttackRadius(float NewAttackRadius)
{
	GetBlackboardComponent()->SetValueAsFloat(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_AttackRadius(), NewAttackRadius);
}

void AEnemyAIController::BlackBoardUpdate_DefendRadius(float NewDefendRadius)
{
	GetBlackboardComponent()->SetValueAsFloat(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_DefendRadius(), NewDefendRadius);
}

void AEnemyAIController::BlackBoardUpdate_PointOfInterest(FVector NewPointOfInterest)
{
	GetBlackboardComponent()->SetValueAsVector(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_PointOfInterest(), NewPointOfInterest);
}

void AEnemyAIController::BlackBoardUpdate_AttackTarget(UObject* NewAttackTarget)
{
	GetBlackboardComponent()->SetValueAsObject(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_AttackTarget(), NewAttackTarget);
}
