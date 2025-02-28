#pragma once

#include "CoreMinimal.h"
#include "ProGameplayAbility.h"
#include "Ability_HitReaction.generated.h"

/** 
 * 피격 시 HitMontage를 재생하고, 재생이 끝나면 어빌리티가 종료되는 예시 클래스 
 */
UCLASS()
class SHOOTERPRO_API UAbility_HitReaction : public UProGameplayAbility
{
	GENERATED_BODY()

public:
	UAbility_HitReaction();

public:
	/** 
	 * GameplayAbility가 활성화될 때 호출됨 
	 * - Hit 이벤트가 발생했을 때 호출되도록 설정 
	 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	/** 
	 * Montage 완료(또는 중단) 시 호출될 콜백 함수들 
	 */
	UFUNCTION()
	void OnMontageFinished();

	UFUNCTION()
	void OnMontageCancelled();

protected:
	/**
	 * 실제로 재생할 히트 반응 애니메이션 몽타주
	 * - 에디터/블루프린트에서 설정할 수 있게 함
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReaction")
	TObjectPtr<UAnimMontage> HitMontage;

	/**
	 * Montage 재생 시 기다리는 태스크(WaitMontage)처럼 사용할 바인딩
	 * - Montage 재생 후 완료/취소를 처리하기 위한 핸들
	 */
	UPROPERTY()
	FActiveGameplayEffectHandle MontageTaskHandle;
};
