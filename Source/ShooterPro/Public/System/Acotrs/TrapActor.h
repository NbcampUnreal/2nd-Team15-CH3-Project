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

	/** 플레이어가 함정 위를 밟았을 때 트리거되는 콜리전 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trap|Components")
	UBoxComponent* TriggerBox;

	/** 함정이 발동될 때 적용할 GameplayEffect (데미지나 디버프 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap|GAS")
	TSubclassOf<UGameplayEffect> TrapGameplayEffect;

	/** 함정 발동 시 재생할 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap|FX")
	USoundBase* TrapSound;

	/** 함정 발동 시 생성할 나이아가라 시스템(불바다 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Trap|FX")
	UNiagaraSystem* FireNiagaraSystem;

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
};
