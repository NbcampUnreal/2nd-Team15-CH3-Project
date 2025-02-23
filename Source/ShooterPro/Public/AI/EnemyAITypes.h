#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"

#include "EnemyAITypes.generated.h"


UENUM(BlueprintType, meta=(Bitmask, UseEnumValuesAsMaskValuesInEditor="true"))
enum class ECombatTriggerFlags : uint8
{
	None = 0,
	Sight = 1 << 0,
	Hearing = 1 << 1,
	Damage = 1 << 2,
	OnSpawn = 1 << 3,
};

/** 
 * 여러 감각 종류를 열거하는 Enum 
 * EAISense는 AI가 감지할 수 있는 감각을 나타냅니다.
 * - Sight: 시각
 * - Hearing: 청각
 * - Damage: 피해 감지
 */
UENUM(BlueprintType)
enum class EAISense : uint8
{
	None, // 감각 없음
	Sight, // 시각
	Hearing, // 청각
	Damage, // 피해
};


UENUM(BlueprintType)
enum class EAIMovementSpeed : uint8
{
	Idle,
	Walking,
	Jogging,
	Sprinting,
};


USTRUCT(BlueprintType)
struct SHOOTERPRO_API FSenseHandle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Handle")
	EAISense MostRecentSense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sense Handle")
	bool bLostStimulus;
};


UENUM(BlueprintType)
enum class EDamageResponse : uint8
{
	None,
	HitReaction,
	Stagger,
	Stun,
	KnockBack
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
	None,
	Melee,
	Projectile,
	Explosion,
	Environment
};


USTRUCT(BlueprintType)
struct SHOOTERPRO_API FDamageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Info")
	float Amount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Info")
	EDamageType DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Info")
	EDamageResponse DamageResponse;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Info")
	bool bShouldDamageInvincible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Info")
	bool bCanBeBlocked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Info")
	bool bCanBeParried;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Info")
	bool bShouldForceInterrupt;
};


USTRUCT(BlueprintType)
struct SHOOTERPRO_API FAttackInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Info")
	AActor* AttackTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Info")
	UAnimMontage* Montage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Info")
	FDamageInfo DamageInfo;
};
