#include "ShooterPro/Public/AI/EnemyAIController.h"

#include "AI/AIGameplayTags.h"

#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

#include "Runtime/AIModule/Classes/BehaviorTree/BehaviorTree.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BlackboardComponent.h"
#include "Runtime/AIModule/Classes/Perception/AIPerceptionComponent.h"

#include "ShooterPro/Public/AI/EnemyAIBase.h"
#include "ShooterPro/Public/AI/EnemyAILog.h"

#include "AI/Components/ProAIBehaviorsComponent.h"
#include "AI/Utility/EnemyAIBluePrintFunctionLibrary.h"


#include "BlackboardKeyType_GameplayTag.h"


AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// AI Perception 컴포넌트를 생성합니다.
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// -- 시야(Sight) 설정 --
	if (UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("AISenseConfig_Sight")))
	{
		SightConfig->SightRadius = 1500.f;
		SightConfig->LoseSightRadius = 2000.f;
		SightConfig->PeripheralVisionAngleDegrees = 60.f;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->SetMaxAge(20.f);
		AIPerception->ConfigureSense(*SightConfig);
	}

	// -- 청각(Hearing) 설정 --
	if (UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("AISenseConfig_Hearing")))
	{
		HearingConfig->HearingRange = 500.f;
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		HearingConfig->SetMaxAge(3.f);
		AIPerception->ConfigureSense(*HearingConfig);
	}

	// -- 피해(Damage) 설정 --
	if (UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("AISenseConfig_Damage")))
	{
		DamageConfig->SetMaxAge(5.f);
		AIPerception->ConfigureSense(*DamageConfig);
	}

	// 주 감각(DominantSense)으로 시야(Sight)를 설정합니다.
	AIPerception->SetDominantSense(UAISense_Sight::StaticClass());
}

void AEnemyAIController::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!DetectionInfoManager)
		DetectionInfoManager = NewObject<UPerceptionManager>(this, UPerceptionManager::StaticClass());

	PossessedAI = Cast<AEnemyAIBase>(InPawn);
	if (!PossessedAI)
	{
		AI_ENEMY_SCREEN_LOG_ERROR(5.0f, "InPawn을 AEnemyAIBase로 캐스팅할 수 없습니다.");
		return;
	}

	if (!IsValid(PossessedAI->BehaviorTree))
	{
		AI_ENEMY_SCREEN_LOG_ERROR(5.0f, "Behavior을 설정해주세요");
		return;
	}

	// AIBehaviorsComponent가 유효하면 공격 및 방어 반경 업데이트
	if (IsValid(PossessedAI->AIBehaviorsComponent))
	{
		RunBehaviorTree(PossessedAI->BehaviorTree);
		
		AIBehaviorComponent = PossessedAI->AIBehaviorsComponent;
		if (AIBehaviorComponent)
			AIBehaviorComponent->InitializeBehavior(this);
	}

	AIPerception->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionForgotten);


}

void AEnemyAIController::OnUnPossess()
{
	AIPerception->OnPerceptionUpdated.RemoveDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.RemoveDynamic(this, &AEnemyAIController::OnTargetPerceptionForgotten);

	Super::OnUnPossess();
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyAIController::UpdatePerception(float DeltaTime)
{
	// 1. 센서의 현재 위치를 모든 DetectionInfo에 업데이트
	if (AIPerception)
	{
		FVector CurrentSensorLocation, CurrentSensorDirection;
		AIPerception->GetLocationAndDirection(CurrentSensorLocation, CurrentSensorDirection);

		// 감지된 액터들에 대해 Sensor 위치 업데이트
		for (TTuple<AActor*, FPerceivedActorEntry>& Entry : DetectionInfoManager->DetectionList)
		{
			for (FPerceivedActorInfo& Info : Entry.Value.PerceivedActorInfos)
			{
				Info.LastSensorLocation = CurrentSensorLocation;
			}
		}
	}

	// 2. 현재 Perception에 의해 감지된 액터들을 가져옵니다.
	TArray<AActor*> KnownPerceivedActors;
	AIPerception->GetKnownPerceivedActors(nullptr, KnownPerceivedActors);

	// 3. 각 액터별로 최신 자극(Stimulus)을 구하고, 해당 액터에 한해서 감각 업데이트를 수행합니다.
	for (AActor* Actor : KnownPerceivedActors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		FActorPerceptionBlueprintInfo ActorInfo;
		if (!AIPerception->GetActorsPerception(Actor, ActorInfo))
		{
			continue;
		}

		// 각 감각별 최신 자극을 액터 단위로 수집합니다.
		FAIStimulus SightStimulus;
		bool bSightFound = false;
		FAIStimulus HearingStimulus;
		bool bHearingFound = false;
		FAIStimulus DamageStimulus;
		bool bDamageFound = false;

		for (const FAIStimulus& Stimulus : ActorInfo.LastSensedStimuli)
		{
			if (!Stimulus.IsValid())
			{
				continue;
			}

			TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
			if (SenseClass == UAISense_Sight::StaticClass())
			{
				SightStimulus = Stimulus;
				bSightFound = true;
			}
			else if (SenseClass == UAISense_Hearing::StaticClass())
			{
				HearingStimulus = Stimulus;
				bHearingFound = true;
			}
			else if (SenseClass == UAISense_Damage::StaticClass())
			{
				DamageStimulus = Stimulus;
				bDamageFound = true;
			}
		}

		// 4. 각 감각별로 유효한 자극이 있다면, 해당 액터와 관련된 DetectionInfo만 업데이트합니다.
		if (bSightFound)
		{
			DetectionInfoManager->TickSenseDetectionsForActor(DeltaTime, EAISense::Sight, SightStimulus, Actor);
		}
		if (bHearingFound)
		{
			DetectionInfoManager->TickSenseDetectionsForActor(DeltaTime, EAISense::Hearing, HearingStimulus, Actor);
		}
		if (bDamageFound)
		{
			DetectionInfoManager->TickSenseDetectionsForActor(DeltaTime, EAISense::Damage, DamageStimulus, Actor);
		}
	}
}


bool AEnemyAIController::CanPerceiveActor(AActor* Actor, EAISense SenseType, FAIStimulus& OutAIStimulus)
{
	FActorPerceptionBlueprintInfo PerceptionInfo;
	AIPerception->GetActorsPerception(Actor, PerceptionInfo);

	// 액터에 대해 마지막으로 감지된 자극들을 순회
	for (FAIStimulus LastSensedStimulus : PerceptionInfo.LastSensedStimuli)
	{
		// 자극에 해당하는 감각 클래스를 가져옴
		TSubclassOf<UAISense> SenseClassForStimulus = UAIPerceptionSystem::GetSenseClassForStimulus(this, LastSensedStimulus);

		bool bFindScene;
		if (SenseType == EAISense::Sight)
			bFindScene = SenseClassForStimulus == UAISense_Sight::StaticClass();
		else if (SenseType == EAISense::Hearing)
			bFindScene = SenseClassForStimulus == UAISense_Hearing::StaticClass();
		else if (SenseType == EAISense::Damage)
			bFindScene = SenseClassForStimulus == UAISense_Damage::StaticClass();
		else
			bFindScene = false;

		// 해당 감각 자극이 발견되면, 자극 정보를 OutAIStimulus에 저장하고 성공 여부 반환
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
	// 두 액터가 UInterface_Damagable 인터페이스를 구현하지 않았다면 오류 로그 출력 후 true 반환 (보수적 판단)
	if (!Actor->Implements<UInterface_EnemyAI>() || !GetPawn()->Implements<UInterface_EnemyAI>())
	{
		// AI_ENEMY_SCREEN_LOG_ERROR(5.0f, "TargetActor 또는 Pawn이 UInterface_EnemyAI를 구현하지 않았습니다. 그래서 서로 적으로 인식됩니다.");
		return false;
	}

	return IInterface_EnemyAI::Execute_GetTeamNumber(Actor) == IInterface_EnemyAI::Execute_GetTeamNumber(GetPawn());
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	for (AActor* UpdatedActor : UpdatedActors)
	{
		if (!IsValid(UpdatedActor) || OnSameTeam(UpdatedActor))
			continue;

		// 해당 액터의 최신 Perception 정보를 가져옵니다.
		FActorPerceptionBlueprintInfo ActorPerceptionInfo;
		if (!AIPerception->GetActorsPerception(UpdatedActor, ActorPerceptionInfo))
			continue;

		// 각 감각별 자극 정보를 확인합니다.
		for (const FAIStimulus& Stimulus : ActorPerceptionInfo.LastSensedStimuli)
		{
			if (!Stimulus.IsValid()) continue;

			// Stimulus에 해당하는 감각 클래스를 결정합니다.
			TSubclassOf<UAISense> SenseClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
			EAISense SenseType = EAISense::None;
			if (SenseClass == UAISense_Sight::StaticClass()) SenseType = EAISense::Sight;
			else if (SenseClass == UAISense_Hearing::StaticClass()) SenseType = EAISense::Hearing;
			else if (SenseClass == UAISense_Damage::StaticClass()) SenseType = EAISense::Damage;
			if (SenseType == EAISense::None) continue;

			// 새로운 자극 또는 감지 상태의 변화가 있을 때, DetectionInfo를 추가 또는 업데이트합니다.
			DetectionInfoManager->AddOrUpdateDetection(GetPawn(), UpdatedActor, SenseType, Stimulus, CurrentTime);
		}
	}
}

void AEnemyAIController::OnTargetPerceptionForgotten(AActor* ForgottenActor)
{
	// 단순히 DetectionInfoManager에서 해당 액터를 제거합니다.
	DetectionInfoManager->ForgetActor(ForgottenActor);
}

void AEnemyAIController::UpdateBlackboard_State(EAIState NewState)
{
	GetBlackboardComponent()->SetValueAsEnum(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_PreviousState(), static_cast<uint8>(GetCurrentState()));
	GetBlackboardComponent()->SetValueAsEnum(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_CurrentState(), static_cast<uint8>(NewState));
}

EAIState AEnemyAIController::GetCurrentState() const
{
	return static_cast<EAIState>(GetBlackboardComponent()->GetValueAsEnum(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_PreviousState()));
}

EAIState AEnemyAIController::GetPreviousState() const
{
	return static_cast<EAIState>(GetBlackboardComponent()->GetValueAsEnum(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_CurrentState()));
}

void AEnemyAIController::UpdateBlackboard_AttackRadius(float NewAttackRadius)
{
	GetBlackboardComponent()->SetValueAsFloat(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_AttackRadius(), NewAttackRadius);
}

void AEnemyAIController::UpdateBlackboard_DefendRadius(float NewDefendRadius)
{
	GetBlackboardComponent()->SetValueAsFloat(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_DefendRadius(), NewDefendRadius);
}

void AEnemyAIController::UpdateBlackboard_StartLocation(FVector NewStartLocation)
{
	GetBlackboardComponent()->SetValueAsVector(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_StartLocation(), NewStartLocation);
}

void AEnemyAIController::UpdateBlackboard_MaxRandRadius(float NewMaxRandRadius)
{
	GetBlackboardComponent()->SetValueAsFloat(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_MaxRangeRadius(), NewMaxRandRadius);
}

void AEnemyAIController::UpdateBlackboard_PointOfInterest(FVector NewPointOfInterest)
{
	GetBlackboardComponent()->SetValueAsVector(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_PointOfInterest(), NewPointOfInterest);
}

void AEnemyAIController::UpdateBlackboard_AttackTarget(UObject* NewAttackTarget)
{
	GetBlackboardComponent()->SetValueAsObject(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_AttackTarget(), NewAttackTarget);
}

void AEnemyAIController::UpdateBlackboard_AttackTarget_ClearValue()
{
	GetBlackboardComponent()->ClearValue(UEnemyAIBluePrintFunctionLibrary::GetBBKeyName_AttackTarget());
}
