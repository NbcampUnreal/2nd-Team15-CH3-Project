#include "AI/Components/AIBehaviorsComponent.h"

#include "AIController.h"
#include "AI/ShooterAIController.h"
#include "AI/Actors/PatrolPath.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"


UAIBehaviorsComponent::UAIBehaviorsComponent(): CurrentState(), DeadAnimation(),
                                                CurrentTransition(), ApproachTransition(),
                                                MeleeAttackTransition(),
                                                RangeAttackTransition(),
                                                StrafeTransition(),
                                                CompanionTransition(),
                                                MeleeAttackAnimation(),
                                                RangeAttackAnimation(nullptr),
                                                RangeReloadAnimation(nullptr), HitAnimation(), EquipWeaponAnimation(nullptr), UnEquipWeaponAnimation(nullptr)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...
	TargetTagCompanion = "Player";

	TargetTags.Add("Player");

	InitialBehavior = EAIBehavior::Idle;
	CurrentState = EAIState::Idle;

	CurrentBehaviorConfig.BehaviorType = EAIBehavior::Idle;
	CurrentBehaviorConfig.MovementSpeedType = EAIMovementState::Idle;
	CurrentBehaviorConfig.OnSenseFoundTarget = EAIBehavior::Idle;
	CurrentBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;

	IdleBehaviorConfig.BehaviorType = EAIBehavior::Idle;
	IdleBehaviorConfig.MovementSpeedType = EAIMovementState::Idle;
	IdleBehaviorConfig.OnSenseFoundTarget = EAIBehavior::Approach;
	IdleBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;

	PatrolBehaviorConfig.BehaviorType = EAIBehavior::Patrol;
	PatrolBehaviorConfig.MovementSpeedType = EAIMovementState::Walk;
	PatrolBehaviorConfig.OnSenseFoundTarget = EAIBehavior::MeleeAttack;
	PatrolBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Patrol;

	ApproachBehaviorConfig.BehaviorType = EAIBehavior::Approach;
	ApproachBehaviorConfig.MovementSpeedType = EAIMovementState::Jog;
	ApproachBehaviorConfig.OnSenseFoundTarget = EAIBehavior::Approach;
	ApproachBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;

	CompanionBehaviorConfig.BehaviorType = EAIBehavior::Companion;
	CompanionBehaviorConfig.MovementSpeedType = EAIMovementState::Jog;
	CompanionBehaviorConfig.OnSenseFoundTarget = EAIBehavior::MeleeAttack;
	CompanionBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Companion;


	RoamingBehaviorConfig.BehaviorType = EAIBehavior::Roaming;
	RoamingBehaviorConfig.MovementSpeedType = EAIMovementState::Walk;
	RoamingBehaviorConfig.OnSenseFoundTarget = EAIBehavior::MeleeAttack;
	RoamingBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;


	RoamingBehaviorConfig.BehaviorType = EAIBehavior::Roaming;
	RoamingBehaviorConfig.MovementSpeedType = EAIMovementState::Walk;
	RoamingBehaviorConfig.OnSenseFoundTarget = EAIBehavior::MeleeAttack;
	RoamingBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;


	MeleeAttackBehaviorConfig.BehaviorType = EAIBehavior::MeleeAttack;
	MeleeAttackBehaviorConfig.MovementSpeedType = EAIMovementState::Walk;
	MeleeAttackBehaviorConfig.OnSenseFoundTarget = EAIBehavior::MeleeAttack;
	MeleeAttackBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;


	RangeAttackBehaviorConfig.BehaviorType = EAIBehavior::RangeAttack;
	RangeAttackBehaviorConfig.MovementSpeedType = EAIMovementState::Walk;
	RangeAttackBehaviorConfig.OnSenseFoundTarget = EAIBehavior::RangeAttack;
	RangeAttackBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;


	StrafeBehaviorConfig.BehaviorType = EAIBehavior::Strafe;
	StrafeBehaviorConfig.MovementSpeedType = EAIMovementState::Walk;
	StrafeBehaviorConfig.OnSenseFoundTarget = EAIBehavior::Strafe;
	StrafeBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;


	HitBehaviorConfig.BehaviorType = EAIBehavior::Hit;
	HitBehaviorConfig.MovementSpeedType = EAIMovementState::Idle;
	HitBehaviorConfig.OnSenseFoundTarget = EAIBehavior::Idle;
	HitBehaviorConfig.OnSenseLoseTarget = EAIBehavior::Idle;
}

void UAIBehaviorsComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...

	AAIController* OwnerAIController = UAIBlueprintHelperLibrary::GetAIController(GetOwner());
	if (IsValid(OwnerAIController))
	{
		OwnerControllerRef = Cast<AAIController>(OwnerAIController);
		OwnerCharacterRef = Cast<ACharacter>(OwnerControllerRef->GetPawn());
	}

	ChangeBehavior(InitialBehavior);
}

void UAIBehaviorsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAIBehaviorsComponent::ChangeBehavior(EAIBehavior NewBehavior)
{
	if (NewBehavior == EAIBehavior::Idle) CurrentBehaviorConfig = IdleBehaviorConfig;
	if (NewBehavior == EAIBehavior::Patrol) CurrentBehaviorConfig = PatrolBehaviorConfig;
	if (NewBehavior == EAIBehavior::Approach) CurrentBehaviorConfig = ApproachBehaviorConfig;
	if (NewBehavior == EAIBehavior::Roaming) CurrentBehaviorConfig = RoamingBehaviorConfig;
	if (NewBehavior == EAIBehavior::MeleeAttack) CurrentBehaviorConfig = MeleeAttackBehaviorConfig;
	if (NewBehavior == EAIBehavior::RangeAttack) CurrentBehaviorConfig = RangeAttackBehaviorConfig;
	if (NewBehavior == EAIBehavior::Strafe) CurrentBehaviorConfig = StrafeBehaviorConfig;
	if (NewBehavior == EAIBehavior::Hit) CurrentBehaviorConfig = HitBehaviorConfig;
	if (NewBehavior == EAIBehavior::Companion) CurrentBehaviorConfig = CompanionBehaviorConfig;

	SetMovementState(CurrentBehaviorConfig.MovementSpeedType);

	UpdateBehaviorKey(NewBehavior);

	if (CurrentBehaviorConfig.BehaviorType == EAIBehavior::Approach) CurrentTransition = ApproachTransition;
	if (CurrentBehaviorConfig.BehaviorType == EAIBehavior::MeleeAttack) CurrentTransition = MeleeAttackTransition;
	if (CurrentBehaviorConfig.BehaviorType == EAIBehavior::RangeAttack) CurrentTransition = RangeAttackTransition;
	if (CurrentBehaviorConfig.BehaviorType == EAIBehavior::Strafe) CurrentTransition = StrafeTransition;
	if (CurrentBehaviorConfig.BehaviorType == EAIBehavior::Companion) CurrentTransition = CompanionTransition;

	if (CurrentBehaviorConfig.BehaviorType == EAIBehavior::MeleeAttack) AcceptanceDistance = MeleeAttackDistance;
	else if (CurrentBehaviorConfig.BehaviorType == EAIBehavior::RangeAttack) AcceptanceDistance = RangeAttackDistance;
	else if (CurrentBehaviorConfig.BehaviorType == EAIBehavior::Companion) AcceptanceDistance = FMath::FRandRange(CompanionAcceptDistanceMin, CompanionAcceptDistanceMax);
	else { AcceptanceDistance = 0.0f; }

	SwitchTransitionBehavior();
}

void UAIBehaviorsComponent::UpdateBehavior(bool bNewCompanion)
{
	if (IsValid(Target))
	{
		ChangeBehavior(bNewCompanion ? EAIBehavior::Companion : CurrentBehaviorConfig.OnSenseFoundTarget);
	}
	else
	{
		ChangeBehavior(CurrentBehaviorConfig.OnSenseLoseTarget);
	}
}

void UAIBehaviorsComponent::SetMovementState(EAIMovementState MovementState)
{
	UCharacterMovementComponent* CharacterMovement = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();

	if (!IsValid(CharacterMovement))
		return;

	if (MovementState == EAIMovementState::Idle) CharacterMovement->MaxWalkSpeed = 0.0f;
	else if (MovementState == EAIMovementState::Walk) CharacterMovement->MaxWalkSpeed = WalkSpeed;
	else if (MovementState == EAIMovementState::Jog) CharacterMovement->MaxWalkSpeed = JogSpeed;
	else if (MovementState == EAIMovementState::RandomWalkOrJog)
	{
		FMath::RandBool() ? CharacterMovement->MaxWalkSpeed = WalkSpeed : CharacterMovement->MaxWalkSpeed = JogSpeed;
	}
}

void UAIBehaviorsComponent::SwitchTransitionBehavior()
{
	float Time = CurrentTransition.DelayTrigger + FMath::FRandRange(0.0f, CurrentTransition.RandomDeviation);
	TransitionBehaviorTimer = UKismetSystemLibrary::K2_SetTimer(this, "StartTransitionBehavior", Time, true);
}

void UAIBehaviorsComponent::StartTransitionBehavior()
{
	for (const FAITransition& Transition : CurrentTransition.Transitions)
	{
		float DistanceTarget = OwnerCharacterRef->GetDistanceTo(Target);

		bool bDistanceComparison = false;
		if (Transition.Comparison == EComparisonMethod::Equal_To) bDistanceComparison = DistanceTarget == Transition.DistanceToTarget;
		if (Transition.Comparison == EComparisonMethod::Not_Equal_To) bDistanceComparison = DistanceTarget != Transition.DistanceToTarget;
		if (Transition.Comparison == EComparisonMethod::Greater_Than_Or_Equal_To) bDistanceComparison = DistanceTarget >= Transition.DistanceToTarget;
		if (Transition.Comparison == EComparisonMethod::Less_Than_Or_Equal_To) bDistanceComparison = DistanceTarget <= Transition.DistanceToTarget;
		if (Transition.Comparison == EComparisonMethod::Greater_Than) bDistanceComparison = DistanceTarget > Transition.DistanceToTarget;
		if (Transition.Comparison == EComparisonMethod::Less_Than) bDistanceComparison = DistanceTarget < Transition.DistanceToTarget;

		if (!bDistanceComparison)
			return;

		int RandChance = FMath::RandRange(1, 100);

		if (Transition.RateChance <= RandChance)
		{
			DoOnceStartTransitionBehavior.Execute([this, Transition]()
			{
				UKismetSystemLibrary::K2_ClearAndInvalidateTimerHandle(this, TransitionBehaviorTimer);
				ChangeBehavior(Transition.NewBehavior);
			});
		}
	}
}

void UAIBehaviorsComponent::SetCombatMode(bool bValue)
{
	bInCombatMode = bValue;

	if (OnCombatChange.IsBound())
		OnCombatChange.Broadcast(bInCombatMode);
}

void UAIBehaviorsComponent::StartEquipOrUnEquipWeapon(bool bEquip)
{
	SetState(EAIState::SwitchCombat);

	UAnimMontage* SelectAnimMontage = bEquip ? EquipWeaponAnimation : UnEquipWeaponAnimation;
	if (IsValid(SelectAnimMontage))
	{
		PlayMontage(SelectAnimMontage);
		float AnimMontageLength = SelectAnimMontage->GetPlayLength();
		UKismetSystemLibrary::K2_SetTimer(this, "ResetState", AnimMontageLength, false);
	}
	else
	{
		SetCombatMode(bEquip);
	}
}

void UAIBehaviorsComponent::SetState(EAIState NewState)
{
	EAIState PrevState = CurrentState;
	CurrentState = NewState;

	if (PrevState != CurrentState)
	{
		if (OnStateChanged.IsBound())
			OnStateChanged.Broadcast(PrevState, NewState);
	}
}

void UAIBehaviorsComponent::ResetState()
{
	if (CurrentState != EAIState::Dead)
	{
		SetState(EAIState::Idle);
	}
}

void UAIBehaviorsComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
	OwnerCharacterRef->GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay, 1.0f, EMontagePlayReturnType::Duration);
}

bool UAIBehaviorsComponent::PlayDeadAnimation(EHitDirection Direction, float& Duration)
{
	SetState(EAIState::Dead);

	UAnimMontage* SelectMontage = nullptr;
	if (Direction == EHitDirection::Front) SelectMontage = DeadAnimation.Front;
	if (Direction == EHitDirection::Back) SelectMontage = DeadAnimation.Back;
	if (Direction == EHitDirection::Right) SelectMontage = DeadAnimation.Right;
	if (Direction == EHitDirection::Left) SelectMontage = DeadAnimation.Left;

	if (IsValid(SelectMontage))
	{
		float DeadAnimLength = SelectMontage->GetPlayLength();
		PlayMontage(SelectMontage);
		UKismetSystemLibrary::K2_SetTimer(this, "ResetState", DeadAnimLength, false);
		Duration = DeadAnimLength;
		return true;
	}


	ResetState();
	Duration = 0.0f;
	return false;
}

void UAIBehaviorsComponent::FoundTarget(AActor* NewTarget, bool bNewCompanion)
{
	Target = NewTarget;

	UpdateTarget(Target);

	UpdateBehavior(bNewCompanion);

	if (IsValid(Target))
	{
		if (OnFoundTarget.IsBound())
			OnFoundTarget.Broadcast();

		if (bInCombatMode)
			return;

		if (bUseEquipUnEquipWeaponAnim)
			StartEquipOrUnEquipWeapon(true);
		else
			SetCombatMode(true);
	}
	else
	{
		if (OnLoseTarget.IsBound())
			OnLoseTarget.Broadcast();

		if (CurrentBehaviorConfig.BehaviorType != EAIBehavior::Companion)
		{
			if (!bInCombatMode)
				return;

			if (bUseEquipUnEquipWeaponAnim)
				StartEquipOrUnEquipWeapon(false);
			else
				SetCombatMode(false);
		}

		if (bIsAiming)
			SetAiming(false);
	}
}


void UAIBehaviorsComponent::DeadRagdoll(const FVector& HitDirection, FName HitBoneName, float DamageImpulse, EHitDirection Direction)
{
	if (CurrentState == EAIState::Dead)
		return;

	if (!bUseDeadRagdoll)
	{
		float Duration;
		PlayDeadAnimation(Direction, Duration);

		float Delay = Duration * 0.8f;
		UKismetSystemLibrary::K2_SetTimer(this, "DeadRagdoll_Internal", Delay, false);
	}
	else
	{
		DeadRagdoll_Internal(HitDirection, HitBoneName, DamageImpulse);
	}
}

FTransform UAIBehaviorsComponent::GetTargetActorTransform()
{
	if (IsValid(Target))
	{
		return Target->GetActorTransform();
	}

	return {};
}

float UAIBehaviorsComponent::MeleeAttack(float& Duration)
{
	if (!CanAttack())
	{
		Duration = 0.0f;
		return false;
	}

	SetState(EAIState::Attacking);

	int LastIndex = MeleeAttackAnimation.Montages.Num() - 1;

	int32 SelectIndex;
	if (MeleeAttackAnimation.bPlayRandomMontage)
	{
		SelectIndex = FMath::RandRange(0, LastIndex);
	}
	else
	{
		SelectIndex = AttackCounter > LastIndex ? LastIndex : AttackCounter;
	}

	UAnimMontage* SelectMontage = MeleeAttackAnimation.Montages[SelectIndex];
	if (!IsValid(SelectMontage))
	{
		ResetState();
		Duration = 0.0f;
		return false;
	}

	PlayMontage(SelectMontage);
	float AttackMontageLength = SelectMontage->GetPlayLength();
	UKismetSystemLibrary::K2_SetTimer(this, "ResetState", AttackMontageLength, false);

	AttackCounter++;
	if (AttackCounter > LastIndex)
		AttackCounter = 0;

	if (OnMeleeAttack.IsBound())
		OnMeleeAttack.Broadcast();

	Duration = AttackMontageLength;
	return true;
}

float UAIBehaviorsComponent::RangeAttack(float& Duration)
{
	if (!CanAttack())
	{
		Duration = 0.0f;
		return false;
	}

	SetState(EAIState::Attacking);

	if (!IsValid(RangeAttackAnimation))
	{
		ResetState();
		Duration = 0.0f;
		return false;
	}

	PlayMontage(RangeAttackAnimation);
	float RangeMontageLength = RangeAttackAnimation->GetPlayLength();
	UKismetSystemLibrary::K2_SetTimer(this, "ResetState", RangeMontageLength, false);

	if (OnRangeAttack.IsBound())
		OnRangeAttack.Broadcast();

	Duration = RangeMontageLength;
	return true;
}

float UAIBehaviorsComponent::ReloadRangeAttack()
{
	if (CanAttack())
	{
		SetState(EAIState::Disabled);

		if (IsValid(RangeReloadAnimation))
		{
			PlayMontage(RangeReloadAnimation);
			float ReloadMontageLength = RangeReloadAnimation->GetPlayLength();
			UKismetSystemLibrary::K2_SetTimer(this, "ResetState", ReloadMontageLength, false);
			return ReloadMontageLength;
		}
		else
		{
			ResetState();
		}
	}

	return 0.0f;
}

float UAIBehaviorsComponent::Hitted(EHitDirection NewHitDirection)
{
	if (CanBeHit())
	{
		SetState(EAIState::Disabled);

		UAnimMontage* SelectMontage = nullptr;
		if (NewHitDirection == EHitDirection::Front) SelectMontage = HitAnimation.Front;
		if (NewHitDirection == EHitDirection::Back) SelectMontage = HitAnimation.Back;
		if (NewHitDirection == EHitDirection::Right) SelectMontage = HitAnimation.Right;
		if (NewHitDirection == EHitDirection::Left) SelectMontage = HitAnimation.Left;

		if (SelectMontage)
		{
			PlayMontage(SelectMontage);
			float HitLength = SelectMontage->GetPlayLength();
			UKismetSystemLibrary::K2_SetTimer(this, "ResetState", HitLength, false);
			return HitLength;
		}


		ResetState();
		return 0.0f;
	}

	return 0.0f;
}

bool UAIBehaviorsComponent::CanAttack()
{
	auto CanAttackState = CurrentState == EAIState::Idle || CurrentState == EAIState::Disabled;
	return bInCombatMode && CanAttackState;
}

bool UAIBehaviorsComponent::CanBeHit()
{
	auto bIsIdleOrNotSwitchingCombat = CurrentState == EAIState::Idle || CurrentState != EAIState::SwitchCombat;
	auto bIsNotAttacking = CurrentState != EAIState::Attacking;

	return bIsIdleOrNotSwitchingCombat && bIsNotAttacking && bUseHitAnimation;
}


void UAIBehaviorsComponent::UpdatePatrolPath()
{
	if (!IsValid(PatrolPath))
		return;

	int32 SplinePointNums = PatrolPath->SplineComponent->GetNumberOfSplinePoints();

	if (bPatrolReverseDirection)
	{
		if (PatrolPointIndex <= 0)
		{
			if (PatrolPath->SplineComponent->IsClosedLoop())
			{
				PatrolPointIndex = SplinePointNums - 1;
			}
			else
			{
				bPatrolReverseDirection = false;
				PatrolPointIndex = 1;
			}
		}
		else
		{
			PatrolPointIndex--;
		}
	}
	else
	{
		if (PatrolPointIndex >= SplinePointNums - 1)
		{
			if (PatrolPath->SplineComponent->IsClosedLoop())
			{
				PatrolPointIndex = 0;
			}
			else
			{
				bPatrolReverseDirection = true;
				PatrolPointIndex = SplinePointNums - 2;
			}
		}
		else
		{
			PatrolPointIndex++;
		}
	}
}

FVector UAIBehaviorsComponent::GetSplinePointLocation(int Index)
{
	if (IsValid(PatrolPath))
	{
		return PatrolPath->SplineComponent->GetLocationAtSplinePoint(Index, ESplineCoordinateSpace::World);
	}

	return {};
}

void UAIBehaviorsComponent::DeadRagdoll_Internal(const FVector& HitDirection, FName HitBoneName, float DamageImpulse)
{
	SetState(EAIState::Dead);

	OwnerCharacterRef->GetCharacterMovement()->SetMovementMode(MOVE_None);
	OwnerCharacterRef->GetCharacterMovement()->StopMovementImmediately();
	OwnerCharacterRef->GetMesh()->SetSimulatePhysics(true);
	OwnerCharacterRef->GetMesh()->SetCollisionProfileName(FName("Ragdoll"));
	OwnerCharacterRef->GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	OwnerCharacterRef->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OwnerCharacterRef->GetMesh()->AddImpulse(HitDirection * DamageImpulse, HitBoneName, true);

	if (OnDead.IsBound())
		OnDead.Broadcast();

	OwnerCharacterRef->SetLifeSpan(DeadLifespan);
}

void UAIBehaviorsComponent::UpdateBehaviorKey(EAIBehavior NewBehavior) const
{
	if (IsValid(OwnerControllerRef))
	{
		if (AShooterAIController* ShooterAIController = Cast<AShooterAIController>(OwnerControllerRef))
		{
			ShooterAIController->BlackBoardUpdate_Behavior(NewBehavior);
			if (OnChangeBehavior.IsBound())
				OnChangeBehavior.Broadcast(NewBehavior);
		}
	}
}

void UAIBehaviorsComponent::UpdateTarget(UObject* NewObject) const
{
	if (IsValid(OwnerControllerRef))
	{
		if (AShooterAIController* ShooterAIController = Cast<AShooterAIController>(OwnerControllerRef))
		{
			ShooterAIController->BlackBoardUpdate_Target(NewObject);
		}
	}
}
