#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AI/EnemyAITypes.h"
#include "GameFramework/Actor.h"
#include "ZombieAudioManager.generated.h"

class USoundBase;
class UAudioComponent;
class USoundConcurrency;

USTRUCT(BlueprintType)
struct FZombieSoundSet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<USoundBase*> Sounds;
};

USTRUCT()
struct FZombieMoanData
{
	GENERATED_BODY()

	UPROPERTY()
	UAudioComponent* AudioComp = nullptr;

	UPROPERTY()
	TWeakObjectPtr<AActor> ZombieActor;
};

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

protected:
	/** 주기적으로 울음소리를 내는 타이머 */
	FTimerHandle MoanTimerHandle;

	/** 타이머 콜백 */
	void HandleZombieMoan();

	/** 실제로 해당 좀비에게 울음소리를 재생 시도 (상태, 쿨타임 등 확인) */
	bool TryPlayMoan(AActor* ZombieActor);

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

	/**
	 * @brief 좀비 울음소리의 "전체 볼륨"을 설정 (0.0 ~ 1.0 범위)
	 *        이미 재생 중인 사운드에도 즉시 반영
	 */
	UFUNCTION(BlueprintCallable, Category="ZombieAudio|Volume")
	void SetZombieMoanVolume(float InVolume);

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
	TMap<EAIState, FZombieSoundSet> StateToSoundsMap;

	/**
	 * 상태별 (MinInterval, MaxInterval)
	 * Idle -> (5,10), Combat -> (2,5) 등
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Interval")
	TMap<EAIState, FVector2D> StateToIntervalMap;

	/** 소리를 재생할 최대 거리(플레이어 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Distance")
	float MaxMoanDistance;

	/** 라인트레이스로 시야가 막혔으면 재생 안 하는 옵션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Distance")
	bool bUseLineTrace;

	/**
	 * @brief 좀비별로 울음 재생 쿨타임을 적용하기 위한 설정값(초)
     *        한 번 울고 나면, "현재시간 + ZombieMoanCooldown" 이후 다시 울 수 있음
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZombieAudio|Cooldown")
	float ZombieMoanCooldown;

	/**
	 * @brief **전체적인 좀비 소리 볼륨** (0.0 ~ 1.0)
	 *        새로 재생될 사운드는 이 값으로 VolumeMultiplier를 적용.
	 *        이미 재생 중인 사운드도 SetZombieMoanVolume() 호출 시 일괄 업데이트 가능.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ZombieAudio|Volume", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ZombieMoanVolume;

protected:
	//-------------------------------------
	// Runtime Data
	//-------------------------------------

	/** 현재 등록된 좀비 목록 */
	UPROPERTY()
	TArray<AActor*> RegisteredZombies;

	/** 현재 재생 중인 오디오 데이터 (오디오컴포넌트 + 어떤 좀비인지) */
	UPROPERTY()
	TArray<FZombieMoanData> ActiveMoansData;

	/**
     * 좀비별로 "다시 울 수 있는 시간"을 기록.
	 *        Key: 좀비 Actor, Value: 이 시간이후로 다시 울 수 있음
	 */
	UPROPERTY()
	TMap<AActor*, float> ZombieToNextMoanTime;
};
