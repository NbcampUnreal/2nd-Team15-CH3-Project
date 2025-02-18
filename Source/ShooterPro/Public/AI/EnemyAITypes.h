#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "EnemyAITypes.generated.h"


UENUM(BlueprintType)
enum class EAIState : uint8
{
	Passive,
	Attacking,
	Frozen,
	Investigating,
	Dead,
	Seeking
};

UENUM(BlueprintType)
enum class EAISense : uint8
{
	None,
	Sight,
	Hearing,
	Damage,
};




//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
// UENUM(BlueprintType)
// enum class ECollisionPart : uint8
// {
// 	None,
// 	Weapon,
// 	RightHand,
// 	LeftHand,
// 	RightFoot,
// 	LeftFoot
// };
//
//
//
//
//
// UENUM(BlueprintType)
// enum class EAIBehavior : uint8
// {
// 	Idle,
// 	Patrol,
// 	Approach,
// 	Roaming,
// 	MeleeAttack,
// 	RangeAttack,
// 	Strafe,
// 	Hit,
// 	Companion
// };
//
// UENUM(BlueprintType)
// enum class EHitDirection : uint8
// {
// 	Front,
// 	Back,
// 	Left,
// 	Right
// };
//
//
// UENUM(BlueprintType)
// enum class EAIMovementState : uint8
// {
// 	Idle,
// 	Walk,
// 	Jog,
// 	RandomWalkOrJog
// };
//
//
// USTRUCT(BlueprintType)
// struct SHOOTERPRO_API FBehaviorBase
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Base")
// 	EAIBehavior BehaviorType =EAIBehavior::Idle;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Base")
// 	EAIMovementState MovementSpeedType = EAIMovementState::Idle;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Base")
// 	EAIBehavior OnSenseFoundTarget=EAIBehavior::Idle;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Base")
// 	EAIBehavior OnSenseLoseTarget=EAIBehavior::Idle;
// };
//
// USTRUCT(BlueprintType)
// struct SHOOTERPRO_API FAITransition
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Transition")
// 	EComparisonMethod Comparison = EComparisonMethod::Less_Than_Or_Equal_To;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Transition")
// 	float DistanceToTarget = 0.0f;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Transition")
// 	float RateChance = 100.0f;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Transition")
// 	EAIBehavior NewBehavior = EAIBehavior::Idle;
// };
//
// USTRUCT(BlueprintType)
// struct SHOOTERPRO_API FBehaviorTransitionSettings
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTransitionSettings")
// 	float DelayTrigger = 3.0f;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTransitionSettings")
// 	float RandomDeviation=0.0f;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BehaviorTransitionSettings")
// 	TArray<FAITransition> Transitions;
// };
//
// USTRUCT(BlueprintType)
// struct SHOOTERPRO_API FDirectionMontage
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Montage")
// 	UAnimMontage* Front=nullptr;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Montage")
// 	UAnimMontage* Back=nullptr;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Montage")
// 	UAnimMontage* Left=nullptr;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction Montage")
// 	UAnimMontage* Right=nullptr;
// };
//
//
// USTRUCT(BlueprintType)
// struct SHOOTERPRO_API FMeleeAttackMontage
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeAttack Montage")
// 	bool bPlayRandomMontage=false;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MeleeAttack Montage")
// 	TArray<UAnimMontage*> Montages;
// };
//
//
// USTRUCT(BlueprintType)
// struct SHOOTERPRO_API FHitCollisionComponent
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit CollisionComponent")
// 	UPrimitiveComponent* Component=nullptr;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit CollisionComponent")
// 	TArray<FName> Sockets;
// };
//
//
// USTRUCT(BlueprintType)
// struct SHOOTERPRO_API FHitActorCollisionComponent
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Actor CollisionComponent")
// 	UPrimitiveComponent* Component;
//
// 	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Actor CollisionComponent")
// 	TArray<AActor*> HitActors;
//
// 	FHitActorCollisionComponent() : Component(nullptr)
// 	{
// 	}
//
// 	FHitActorCollisionComponent(UPrimitiveComponent* InComponent, const TArray<AActor*>& InHitActors) : Component(InComponent), HitActors(InHitActors)
// 	{
// 	}
// };
