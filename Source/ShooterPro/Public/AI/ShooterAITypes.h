#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "ShooterAITypes.generated.h"


UENUM(BlueprintType)
enum class ECollisionPart : uint8
{
	None,
	Weapon,
	RightHand,
	LeftHand,
	RightFoot,
	LeftFoot
};


UENUM(BlueprintType)
enum class EAIState : uint8
{
	Idle,
	Disabled,
	Attacking,
	Dead,
	SwitchCombat
};


UENUM(BlueprintType)
enum class EAIBehavior : uint8
{
	Idle,
	Patrol,
	Approach,
	Roaming,
	MeleeAttack,
	RangeAttack,
	Strafe,
	Hit,
	Companion
};

UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	Front,
	Back,
	Left,
	Right
};


UENUM(BlueprintType)
enum class EAIMovementState : uint8
{
	Idle,
	Walk,
	Jog,
	RandomWalkOrJog
};


USTRUCT(BlueprintType)
struct SHOOTERPRO_API FBehaviorBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Base")
	EAIBehavior BehaviorType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Base")
	EAIMovementState MovementSpeedType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Base")
	EAIBehavior OnSenseFoundTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Base")
	EAIBehavior OnSenseLoseTarget;
};

USTRUCT(BlueprintType)
struct SHOOTERPRO_API FAITransition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Transition")
	EComparisonMethod Comparison = EComparisonMethod::Less_Than_Or_Equal_To;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Transition")
	float DistanceToTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Transition")
	float RateChance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Transition")
	EAIBehavior NewBehavior = EAIBehavior::Idle;
};

USTRUCT(BlueprintType)
struct SHOOTERPRO_API FBehaviorTransitionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTransitionSettings")
	float DelayTrigger = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTransitionSettings")
	float RandomDeviation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTransitionSettings")
	TArray<FAITransition> Transitions;
};

USTRUCT(BlueprintType)
struct SHOOTERPRO_API FDirectionMontage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Montage")
	UAnimMontage* Front;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Montage")
	UAnimMontage* Back;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Montage")
	UAnimMontage* Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Montage")
	UAnimMontage* Right;
};


USTRUCT(BlueprintType)
struct SHOOTERPRO_API FMeleeAttackMontage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeAttack Montage")
	bool bPlayRandomMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeAttack Montage")
	TArray<UAnimMontage*> Montages;
};


USTRUCT(BlueprintType)
struct SHOOTERPRO_API FHitCollisionComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit CollisionComponent")
	UPrimitiveComponent* Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit CollisionComponent")
	TArray<FName> Sockets;
};


USTRUCT(BlueprintType)
struct SHOOTERPRO_API FHitActorCollisionComponent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Actor CollisionComponent")
	UPrimitiveComponent* Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Actor CollisionComponent")
	TArray<AActor*> HitActors;
};