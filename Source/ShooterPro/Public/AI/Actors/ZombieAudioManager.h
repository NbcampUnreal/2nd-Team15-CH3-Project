#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ZombieAudioManager.generated.h"


class USoundBase;
class UAudioComponent;

/**
 * @brief 블루프린트/에디터에서 편하게 쓰기 위한 구조체.
 *        내부에 여러 SoundBase 배열을 담을 수 있다.
 */
USTRUCT(BlueprintType)
struct FZombieSoundSet
{
	GENERATED_BODY()

	/** 상태별로 재생할 사운드 목록 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USoundBase*> Sounds;
};


/**
 * @brief 여러 좀비가 가지고 있는 "AI 상태(GameplayTag)"를 
 *        각 좀비의 AEnemyAIController::GetCurrentStateTag()에서 가져와
 *        Idle/Combat 등 사운드를 재생하되, 동시재생 개수나 거리 등을 제한
 */
UCLASS()
class SHOOTERPRO_API AZombieAudioManager : public AActor
{
	GENERATED_BODY()

public:
	AZombieAudioManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	/** 주기적으로 울음소리를 내는 타이머 */
	FTimerHandle MoanTimerHandle;

	/** 타이머 콜백: 동시재생 한도 확인 후, 한 마리 좀비를 골라 사운드 재생 */
	void HandleZombieMoan();

	/** 실제로 무작위 좀비를 골라 상태별 사운드를 재생 */
	void PlayRandomMoanFromZombie();

	/** 상태별 (MinInterval, MaxInterval)에 따라 다음 타이머 설정 */
	void SetNextMoanTimer();

	/** 가장 가까운 플레이어 Pawn 찾기 */
	APawn* GetClosestPlayerPawn(const FVector& FromLocation) const;

public:
	/** 좀비 등록(스폰 시) */
	UFUNCTION(BlueprintCallable, Category="ZombieAudio|TagBased")
	void RegisterZombie(AActor* ZombieActor);

	/** 좀비 해제(죽음, 풀링 반환 등) */
	UFUNCTION(BlueprintCallable, Category="ZombieAudio|TagBased")
	void UnregisterZombie(AActor* ZombieActor);

protected:
	//-------------------------------------
	// Settings
	//-------------------------------------

	/** 동시에 재생할 수 있는 소리 개수 한도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Concurrency")
	int32 MaxConcurrentMoans;

	/** 여러 Concurrency를 적용하고 싶을 때 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Concurrency")
	TArray<USoundConcurrency*> ConcurrencyList;

	/**
	 * "상태 -> FZombieSoundSet"
	 * 예) AIState_Idle -> [IdleSounds...]
	 *     AIState_Combat -> [CombatGrowls...]
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Sounds")
	TMap<FGameplayTag, FZombieSoundSet> StateToSoundsMap;

	/**
	 * 상태별 (MinInterval, MaxInterval)
	 * Idle -> (5,10), Combat -> (2,5) 등
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Interval")
	TMap<FGameplayTag, FVector2D> StateToIntervalMap;

	/** 소리를 재생할 최대 거리(플레이어 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Distance")
	float MaxMoanDistance;

	/** 라인트레이스로 시야가 막혔으면 재생 안 하는 옵션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Distance")
	bool bUseLineTrace;

protected:
	//-------------------------------------
	// Runtime Data
	//-------------------------------------

	/**
	 * 현재 등록된 좀비 목록
	 * (상태 값은 여기서 저장하지 않음 → 
	 *  대신 각 좀비의 AEnemyAIController->GetCurrentStateTag() 사용)
	 */
	UPROPERTY()
	TArray<AActor*> RegisteredZombies;

	/** 현재 재생 중인 AudioComponent (Tick에서 종료된 것 정리) */
	UPROPERTY()
	TArray<UAudioComponent*> ActiveMoans;
};