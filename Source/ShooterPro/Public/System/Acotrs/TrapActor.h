#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "Components/BoxComponent.h"

// Niagara 관련 헤더
#include "NiagaraSystem.h"
#include "TrapActor.generated.h"

class UGameplayEffect;
class USoundBase;

/**
 * ATrapActor
 * - 박스 콜리전을 이용해 플레이어(또는 캐릭터 등)가 밟았을 때 트리거를 발생시키는 예시
 * - GAS를 활용해 GameplayEffect나 Cue를 발동할 수 있도록 구성
 */
UCLASS()
class SHOOTERPRO_API ATrapActor : public AActor
{
	GENERATED_BODY()

public:
	ATrapActor();

protected:
	virtual void BeginPlay() override;

	/** 트랩의 기준이 되는 SceneComponent (루트) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trap|Components")
	USceneComponent* SceneComponent;

	/** 플레이어가 함정 위를 밟았을 때 트리거되는 콜리전 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trap|Components")
	UBoxComponent* TriggerBox;

	/** 
	 * 오버랩된 액터가 여기서 지정한 클래스(또는 자식 클래스)일 때만 트리거 발동
	 * 예: AMyCharacter 클래스 지정 시 해당 클래스 또는 파생 클래스만 함정 발동 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap|Config")
	TSubclassOf<AActor> OverlapFilterClass;

	/** 함정이 발동될 때 적용할 GameplayEffect (데미지나 디버프 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap|GAS")
	TSubclassOf<UGameplayEffect> TrapGameplayEffect;

	/** 함정 발동 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap|FX")
	USoundBase* TrapSound;

	/** 함정 발동 시 생성할 나이아가라 시스템(불바다 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap|FX")
	UNiagaraSystem* FireNiagaraSystem;

	/** 
	 * 함정이 오버랩되었을 때 파괴할지 여부 
	 * - true면 트랩이 오버랩 즉시 혹은 일정 시간 후 파괴됨 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap|Destroy")
	bool bDestroyOnOverlap;

	/**
	 * 함정 파괴 지연 시간(초)
	 * - bDestroyOnOverlap가 true이고 0보다 큰 값이면 
	 *   해당 시간 후에 트랩이 사라짐 
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trap|Destroy", meta = (EditCondition = "bDestroyOnOverlap", ClampMin = "0.0"))
	float DestroyDelay;

public:
	/** 함정 사운드 재생 함수 (블루프린트에서 호출 가능) */
	UFUNCTION(BlueprintCallable, Category="Trap")
	void PlayTrapSound();

	/** 나이아가라 이펙트 스폰 함수 (블루프린트에서 호출 가능) */
	UFUNCTION(BlueprintCallable, Category="Trap")
	void SpawnNiagaraFX();

	/**
	 * 함정 GameplayEffect 적용 함수 (블루프린트에서 호출 가능)
	 * @param TargetActor - 효과를 적용할 대상 액터
	 */
	UFUNCTION(BlueprintCallable, Category="Trap")
	void ApplyTrapEffect(AActor* TargetActor);

protected:
	/** 콜리전 처리 함수 */
	UFUNCTION()
	void OnTrapOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/**
	 * 블루프린트에서 오버라이드할 수 있는 이벤트(추가 처리가 필요한 경우)
	 * C++에서 구현 없이 선언만 해두면 블루프린트에서 구현 가능 
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void K2_OnTrapOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
