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
