#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/ProGameplayAbility.h"
#include "GameplayAbility_LongRangeProjectile.generated.h"

class AEnemyProjectile;

/**
 * 던지기(투사체 발사) 어빌리티 예시
 */
UCLASS()
class SHOOTERPRO_API UGameplayAbility_LongRangeProjectile : public UProGameplayAbility
{
	GENERATED_BODY()

protected:
	// 어빌리티 활성화: 애니메이션 재생 + 이벤트 대기
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	// 실제 투사체를 스폰하는 로직 (ThrowEventTag 이벤트가 들어올 때 호출)
	UFUNCTION()
	void SpawnProjectileFromEvent(FGameplayTag EventTag, FGameplayEventData EventData);

	// 투사체 생성 함수 (직접 스폰 로직)
	void DoSpawnProjectile();

	UFUNCTION()
	void OnMontageCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void OnMontageBlendOut(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void OnMontageInterrupted(FGameplayTag EventTag, FGameplayEventData EventData);

	UFUNCTION()
	void OnMontageCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

public:
	// 몬스터가 사용할 "던지기" 애니메이션 몽타주
	UPROPERTY(EditDefaultsOnly, Category="Ability|Animation")
	UAnimMontage* ThrowMontage;

	// 몽타주에서 투사체를 실제로 스폰할 때 사용할 이벤트 태그(Anim Notify에서 발생)
	UPROPERTY(EditDefaultsOnly, Category="Ability|Animation")
	FGameplayTag ThrowEventTag;

	// 발사할 투사체 클래스
	UPROPERTY(EditDefaultsOnly, Category="Ability|Projectile")
	TSubclassOf<AEnemyProjectile> ProjectileClass;

	// 투사체에 적용할 디버프 효과
	UPROPERTY(EditDefaultsOnly, Category="Ability|Effect")
	TSubclassOf<UGameplayEffect> DebuffEffectClass;

	// 투사체를 발사할 소켓 이름 (예: 캐릭터 메쉬 손/입 소켓)
	UPROPERTY(EditDefaultsOnly, Category="Ability|Projectile")
	FName MuzzleSocketName = TEXT("MuzzleSocket");

	// 실제 던질 때 위로 들어줄 각도 (SuggestProjectileVelocity 사용 시 크게 의미 없을 수 있음)
	// UPROPERTY(EditDefaultsOnly, Category="Ability|Projectile")
	// float ThrowPitchOffset = 30.0f;

private:
	// EndAbility()에 필요한 정보를 임시 저장
	FGameplayAbilitySpecHandle CachedHandle;
	const FGameplayAbilityActorInfo* CachedActorInfo = nullptr;
	FGameplayAbilityActivationInfo CachedActivationInfo;
};
