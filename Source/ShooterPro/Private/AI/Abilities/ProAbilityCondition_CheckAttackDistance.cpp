#include "AI/Abilities/ProAbilityCondition_CheckAttackDistance.h"

#include "AI/Components/AIBehaviorsComponent.h"


UProAbilityCondition_CheckAttackDistance::UProAbilityCondition_CheckAttackDistance()
{
}

bool UProAbilityCondition_CheckAttackDistance::CheckCondition(const UProGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle,
                                                              const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		// 아바타가 없으면 조건 불충족
		return false;
	}

	// AvatarActor에서 UAIBehaviorsComponent를 찾아서 AttackTarget 확인
	AActor* AvatarActor = ActorInfo->AvatarActor.Get();
	if (UAIBehaviorsComponent* BehaviorsComp = AvatarActor->FindComponentByClass<UAIBehaviorsComponent>())
	{
		AActor* TargetActor = BehaviorsComp->AttackTarget;
		if (!IsValid(TargetActor))
		{
			// 공격 대상이 없으면 false
			return false;
		}

		// 거리 계산
		const float DistanceSqr = FVector::DistSquared(AvatarActor->GetActorLocation(), TargetActor->GetActorLocation());
		const float MaxDistSqr = MaxDistance * MaxDistance;

		// 거리 <= MaxDistance 이면 조건 충족
		if (DistanceSqr <= MaxDistSqr)
		{
			return true;
		}
	}

	// 기본: 불충족
	return false;
}
