#pragma once

#include "CoreMinimal.h"
#include "AI/ShooterAITypes.h"
#include "AI/Utility/ShooterProUtility.h"
#include "Components/ActorComponent.h"
#include "AIBehaviorsComponent.generated.h"


class APatrolPath;
class AAIController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttack);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFoundTarget);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDead);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatChange, bool, bIsInCombat);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeBehavior, EAIBehavior, NewBehavior);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStateChanged, EAIState, PreviousState, EAIState, NewState);


UCLASS(ClassGroup=("AI"), meta=(BlueprintSpawnableComponent))
class SHOOTERPRO_API UAIBehaviorsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAIBehaviorsComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void ChangeBehavior(EAIBehavior NewBehavior);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void UpdateBehavior(bool bNewCompanion);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void SetMovementState(EAIMovementState MovementState);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void SwitchTransitionBehavior();

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void StartTransitionBehavior();

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void SetCombatMode(bool bValue);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void StartEquipOrUnEquipWeapon(bool bEquip);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void SetState(EAIState NewState);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void ResetState();

	UFUNCTION(BlueprintPure, Category="AI Behaviors")
	EAIState GetState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void PlayMontage(UAnimMontage* MontageToPlay);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	bool PlayDeadAnimation(EHitDirection Direction, float& Duration);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void SetAiming(bool bNewAim) { bIsAiming = bNewAim; }

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void FoundTarget(AActor* NewTarget, bool bNewCompanion);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors")
	void DeadRagdoll(const FVector& HitDirection, FName HitBoneName, float DamageImpulse, EHitDirection Direction);


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ TARGET ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UFUNCTION(BlueprintPure, Category="AI Behaviors|Target")
	AActor* GetTargetActor() { return Target; }

	UFUNCTION(BlueprintPure, Category="AI Behaviors|Target")
	FTransform GetTargetActorTransform();


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ACTION ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UFUNCTION(BlueprintCallable, Category="AI Behaviors|Action")
	float MeleeAttack(float& Duration);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors|Action")
	float RangeAttack(float& Duration);

	UFUNCTION(BlueprintCallable, Category="AI Behaviors|Action")
	float ReloadRangeAttack();

	UFUNCTION(BlueprintCallable, Category="AI Behaviors|Action")
	float Hitted(EHitDirection NewHitDirection);


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ CONDITION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UFUNCTION(BlueprintPure, Category="AI Behaviors|Condition")
	bool CanAttack();

	UFUNCTION(BlueprintPure, Category="AI Behaviors|Condition")
	bool IsInCombatMode() const { return bInCombatMode; }

	UFUNCTION(BlueprintPure, Category="AI Behaviors|Condition")
	bool CanBeHit();


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PATROL~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UFUNCTION(BlueprintCallable, Category="AI Behaviors|Patrol")
	void UpdatePatrolPath();

	UFUNCTION(BlueprintPure, Category="AI Behaviors|Patrol")
	FVector GetSplinePointLocation(int Index);

protected:
	UFUNCTION()
	void DeadRagdoll_Internal(const FVector& HitDirection, FName HitBoneName, float DamageImpulse);

protected:
	void UpdateBehaviorKey(EAIBehavior NewBehavior) const;
	void UpdateTarget(UObject* NewObject) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors")
	EAIBehavior InitialBehavior;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors")
	EAIState CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors")
	TArray<FName> TargetTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors")
	float WalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors")
	float JogSpeed = 400.0f;

	UPROPERTY( BlueprintReadWrite, Category="AI Behaviors")
	int CurrentAmountRangeAttack =0;

	UPROPERTY(BlueprintReadWrite, Category="AI Behaviors")
	bool bIsAiming = false;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors")
	FDirectionMontage DeadAnimation;
	

	UPROPERTY(BlueprintReadWrite, Category="AI Behaviors")
	float AcceptanceDistance = 50.0f;


	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase CurrentBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase IdleBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase PatrolBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase ApproachBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase RoamingBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase MeleeAttackBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase RangeAttackBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase StrafeBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase HitBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorBase CompanionBehaviorConfig;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorTransitionSettings CurrentTransition;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorTransitionSettings ApproachTransition;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorTransitionSettings MeleeAttackTransition;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorTransitionSettings RangeAttackTransition;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorTransitionSettings StrafeTransition;

	UPROPERTY(BlueprintReadOnly, Category="AI Behaviors")
	FBehaviorTransitionSettings CompanionTransition;


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ REFERENCES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(BlueprintReadWrite, Category="AI Behaviors|Reference")
	TObjectPtr<AAIController> OwnerControllerRef;

	UPROPERTY(BlueprintReadWrite, Category="AI Behaviors|Reference")
	TObjectPtr<ACharacter> OwnerCharacterRef;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Patrol ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ai Behaviors|Patrol")
	bool bPatrolReverseDirection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ai Behaviors|Patrol")
	int32 PatrolPointIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ai Behaviors|Patrol")
	TObjectPtr<APatrolPath> PatrolPath;


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Combat ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
private:
	bool bInCombatMode = false;

	UPROPERTY(BlueprintReadWrite, Category="AI Behaviors|Combat", meta=(AllowPrivateAccess="true"))
	int32 AttackCounter = 0;
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Melee ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Melee")
	FMeleeAttackMontage MeleeAttackAnimation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Melee")
	float MeleeAttackDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Melee")
	float MeleeNextAttackDelay = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Melee")
	float MeleeAttackDamage = 10.0f;


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Range ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Range")
	UAnimMontage* RangeAttackAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Range")
	UAnimMontage* RangeReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Range")
	float RangeAttackDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Range")
	float RangeNextAttackDelay = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Range")
	float RangeAttackDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Range")
	int32 RangeAttackAmmunition = 0;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Hit ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Hit")
	bool bUseHitAnimation = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Hit")
	FDirectionMontage HitAnimation;

	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Equipment ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Equipment")
	bool bUseEquipUnEquipWeaponAnim = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors")
	UAnimMontage* EquipWeaponAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors")
	UAnimMontage* UnEquipWeaponAnimation;
	

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Companion ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Companion")
	float CompanionAcceptDistanceMin = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Companion")
	float CompanionAcceptDistanceMax = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Companion")
	FName TargetTagCompanion;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Roaming ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Roaming")
	float RoamingWaitTime = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Roaming")
	float RoamingWaitTimeDeviation = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Roaming")
	float RoamingMinimumDistance = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI Behaviors|Roaming")
	float RoamingMaximumDistance = 1000.0f;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Dead ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(BlueprintReadWrite, Category="AI Behaviors|Dead")
	bool bUseDeadRagdoll = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="AI Behaviors|Dead")
	float DeadLifespan = 5.0f;


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DELEGATES ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Delegate")
	FOnDead OnDead;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Delegate")
	FOnAttack OnMeleeAttack;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Delegate")
	FOnAttack OnRangeAttack;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Delegate")
	FOnFoundTarget OnFoundTarget;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Delegate")
	FOnFoundTarget OnLoseTarget;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Delegate")
	FOnCombatChange OnCombatChange;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Delegate")
	FOnChangeBehavior OnChangeBehavior;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category="Delegate")
	FOnStateChanged OnStateChanged;


private:
	FDoOnceStruct DoOnceStartTransitionBehavior;

	FTimerHandle TransitionBehaviorTimer;

	UPROPERTY(BlueprintReadWrite, Category="AI Behaviors", meta=(AllowPrivateAccess="true"))
	TObjectPtr<AActor> Target;
};
