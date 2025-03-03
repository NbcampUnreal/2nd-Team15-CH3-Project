#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"
#include "GameplayTagContainer.h"

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


/**
 * @brief 어빌리티가 끝날 때 Broadcast하는 메시지에 담길 페이로드 구조체
 */
USTRUCT(BlueprintType)
struct SHOOTERPRO_API FEnemyAbilityEndedPayload
{
	GENERATED_BODY()

public:
	/** 끝난 어빌리티의 이름 (예: "GA_Fireball") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Ended")
	FString EndedAbilityName;

	/** 끝난 어빌리티가 가지고 있던 대표 GameplayTag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Ended")
	FGameplayTag EndedAbilityTag;

	/** 어빌리티가 종료된 시점(월드 시간 등), 필요에 따라 기록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Ended")
	float EndedTime;

	/** 종료된 Ability를 소유하던 Actor (AI 캐릭터나 Player 등). 불필요 시 제거 가능 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Ended")
	TWeakObjectPtr<AActor> AbilityOwner;

	/** 그 외, 필요한 필드가 있다면 자유롭게 추가 */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ability Ended")
	// int32 SomeAdditionalValue = 0;
	
	/** 기본 생성자 */
	FEnemyAbilityEndedPayload()
		: EndedTime(0.f)
	{
	}
};