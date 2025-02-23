#include "AI/Components/DamageSystemComponent.h"

UDamageSystemComponent::UDamageSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// 추가 초기화 코드 (필요 시 추가)
}

void UDamageSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// 초기화 코드 추가 (필요 시)
}

void UDamageSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 매 프레임 수행할 로직 (필요 시 추가)
}

bool UDamageSystemComponent::TakeDamage(FDamageInfo DamageInfo, AActor* DamageCauser)
{
	// 사망하지 않았고, 무적 상태가 아니거나 무적 상태에도 데미지가 적용되어야 하는 경우
	if (!bIsDead && (!bIsInvincible || DamageInfo.bShouldDamageInvincible))
	{
		// 데미지 블록 처리
		if (bIsBlocking && DamageInfo.bCanBeBlocked)
		{
			// OnBlocked 델리게이트가 바인딩 되어 있다면 호출
			if (OnBlocked.IsBound())
				OnBlocked.Broadcast(DamageInfo.bCanBeParried, DamageCauser);

			// 블록된 경우 데미지 적용 실패
			return false;
		}

		// 데미지 적용
		Health -= DamageInfo.Amount;
		// 체력이 0 이하이면 사망 처리
		if (Health <= 0.0f)
		{
			bIsDead = true;
			// OnDeath 델리게이트 호출 (사망 이벤트 알림)
			if (OnDeath.IsBound())
				OnDeath.Broadcast();

			return true;
		}

		// 데미지에 의한 행동 인터럽트 처리
		if (DamageInfo.bShouldForceInterrupt || bIsInterruptible)
		{
			// OnDamageResponse 델리게이트가 바인딩 되어 있다면 호출
			if (OnDamageResponse.IsBound())
				OnDamageResponse.Broadcast(DamageInfo.DamageResponse, DamageCauser);
		}

		return true;
	}

	// 데미지가 적용되지 않은 경우 (이미 사망했거나 무적 상태)
	return false;
}

float UDamageSystemComponent::Heal(float Amount)
{
	// 액터가 사망하지 않았으면 회복하지 않음
	if (!bIsDead)
		return 0.0f;

	// 회복 적용 후, 최대 체력을 초과하지 않도록 클램핑
	float ModifyHealthValue = Health + Amount;
	Health = FMath::Clamp(ModifyHealthValue, 0.0f, MaxHealth);
	return Health;
}

bool UDamageSystemComponent::ReserveAttackToken(int Amount)
{
	if (AttackTokensCount >= Amount)
	{
		AttackTokensCount -= Amount;
		return true;
	}

	return false;
}

void UDamageSystemComponent::ReturnAttackToken(int Amount)
{
	AttackTokensCount += Amount;
}
