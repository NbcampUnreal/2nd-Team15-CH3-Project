#include "ShooterPro/Public/AI/ShooterAIController.h"


#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

#include "Runtime/AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "Runtime/AIModule/Classes/Perception/AIPerceptionComponent.h"


#include "ShooterPro/Public/AI/ShooterAIBase.h"
#include "ShooterPro/Public/AI/ShooterAILog.h"
#include "ShooterPro/Public/Character/Interfaces/Interface_CharacterStatus.h"

#include "AI/Components/AIBehaviorsComponent.h"
#include "AI/Utility/ShooterAIBluePrintFunctionLibrary.h"


AShooterAIController::AShooterAIController(): bIsInCombat(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;


	//  AIPerceptionComponent 생성 --
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// Sight 설정
	{
		UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISenseConfig_Sight"));
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->SetMaxAge(5.0f);
		AIPerception->ConfigureSense(*SightConfig);
		AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	}

	// Hearing 설정
	{
		UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("AISenseConfig_Hearing"));

		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->SetMaxAge(10.0f);
		AIPerception->ConfigureSense(*HearingConfig);
	}

	// Damage 설정
	{
		UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("AISenseConfig_Damage_2"));
		DamageConfig->SetMaxAge(10.0f);
		AIPerception->ConfigureSense(*DamageConfig);
	}
}

void AShooterAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AIPossessed = Cast<AShooterAIBase>(InPawn);
	if (!AIPossessed)
	{
		SHOOTERAI_LOG_ERROR("InPawn 을 AShooterAIBase 로 캐스팅할 수 없습니다.");
		return;
	}

	if (IsValid(AIPossessed->BehaviorTree))
	{
		RunBehaviorTree(AIPossessed->BehaviorTree);

		if (IsValid(AIPossessed->AIBehaviorsComponent))
		{
			UpdateTimerHandle = UKismetSystemLibrary::K2_SetTimer(this, "UpdateTarget", 1.0f, true);

			PossessedBehaviorsComp = AIPossessed->AIBehaviorsComponent;
			PossessedBehaviorsComp->OnCombatChange.AddDynamic(this, &AShooterAIController::OnCombatChanged);
		}
	}
}

void AShooterAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShooterAIController::UpdateTarget()
{
	UpdateSenseToTarget();
}

void AShooterAIController::UpdateSenseToTarget()
{
	TArray<AActor*> ActorsFound;
	TArray<AActor*> TargetEnemies;

	TArray<AActor*> KnownActors;
	AIPerception->GetKnownPerceivedActors(nullptr, KnownActors);

	for (AActor* KnownActor : KnownActors)
	{
		if (KnownActor && KnownActor->Implements<UInterface_CharacterStatus>())
		{
			if (IInterface_CharacterStatus::Execute_IsAlive(KnownActor))
			{
				ActorsFound.Add(KnownActor);
			}
		}
	}


	for (AActor* FoundActor : ActorsFound)
	{
		for (const FName& TargetTag : PossessedBehaviorsComp->TargetTags)
		{
			if (FoundActor->Tags.IsValidIndex(0))
			{
				if (TargetTag == FoundActor->Tags[0])
				{
					TargetEnemies.AddUnique(FoundActor);
				}
			}
		}
	}

	if (!TargetEnemies.IsEmpty())
	{
		FVector Origin = GetPawn()->GetActorLocation();
		float NearestDistance;
		AActor* NearestTarget = UGameplayStatics::FindNearestActor(Origin, TargetEnemies, NearestDistance);
		SetTarget(NearestTarget);
	}
	else
	{
		if (PossessedBehaviorsComp->CurrentBehaviorConfig.BehaviorType == EAIBehavior::Companion)
		{
			UpdateSenseToTargetCompanion();
		}
		else
		{
			SetTarget(nullptr);
		}
	}
}

void AShooterAIController::UpdateSenseToTargetCompanion()
{
	TArray<AActor*> ActorsFound;
	TArray<AActor*> TargetEnemies;

	TArray<AActor*> KnownActors;
	AIPerception->GetKnownPerceivedActors(nullptr, KnownActors);

	for (AActor* KnownActor : KnownActors)
	{
		if (KnownActor && KnownActor->Implements<UInterface_CharacterStatus>())
		{
			if (IInterface_CharacterStatus::Execute_IsAlive(KnownActor))
			{
				ActorsFound.Add(KnownActor);
			}
		}
	}

	for (AActor* FoundActor : ActorsFound)
	{
		if (FoundActor->Tags.IsValidIndex(0))
		{
			if (PossessedBehaviorsComp->TargetTagCompanion == FoundActor->Tags[0])
			{
				TargetEnemies.AddUnique(FoundActor);
			}
		}
	}


	if (!TargetEnemies.IsEmpty())
	{
		FVector Origin = GetPawn()->GetActorLocation();
		float NearestDistance;
		AActor* NearestTarget = UGameplayStatics::FindNearestActor(Origin, TargetEnemies, NearestDistance);
		SetTarget(NearestTarget, true);
	}
	else
	{
		SetTarget(nullptr);
	}
}

void AShooterAIController::SetTarget(AActor* NewTarget, bool Companion)
{
	if (NewTarget != Target)
	{
		Target = NewTarget;
		PossessedBehaviorsComp->FoundTarget(NewTarget, Companion);
	}
}

void AShooterAIController::OnCombatChanged(bool bNewIsCombat)
{
	BlackBoardUpdate_IsInCombat(bNewIsCombat);
}

void AShooterAIController::BlackBoardUpdate_IsInCombat(bool bNewIsCombat)
{
	if (bNewIsCombat != bIsInCombat)
	{
		bIsInCombat = bNewIsCombat;
		GetBlackboardComponent()->SetValueAsBool(UShooterAIBluePrintFunctionLibrary::GetBBKeyName_InCombat(), bIsInCombat);
	}
}

void AShooterAIController::BlackBoardUpdate_Behavior(EAIBehavior NewBehavior)
{
	GetBlackboardComponent()->SetValueAsEnum(UShooterAIBluePrintFunctionLibrary::GetBBKeyName_Behavior(), static_cast<uint8>(NewBehavior));
}

void AShooterAIController::BlackBoardUpdate_Target(UObject* NewObject)
{
	GetBlackboardComponent()->SetValueAsObject(UShooterAIBluePrintFunctionLibrary::GetBBKeyName_Behavior(), NewObject);
}
