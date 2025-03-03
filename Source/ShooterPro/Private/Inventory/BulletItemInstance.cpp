// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/BulletItemInstance.h"

#include "Inventory/ProBulletBase.h"

UBulletItemInstance::UBulletItemInstance()
{
	ProjectileActorClass =  AProBulletBase::StaticClass();
}

AProBulletBase* UBulletItemInstance::RequestBullet(TSubclassOf<AProBulletBase> BulletClass)
{
	// 1) BulletPool 배열에서 "비활성화된" Bullet을 찾아서 반환
	for (AProBulletBase* Bullet : BulletPool)
	{
		if (IsValid(Bullet) && Bullet->IsHidden()) // 혹은 Bullet->bIsActive 플래그 등
		{
			// 재활성화 하기 전에 원하는 초기화
			return Bullet;
		}
	}

	// 2) 없으면 새로 스폰
	if (GetWorld() && *BulletClass)
	{
		//ToDo:여기 수정하셔야 할수도
		UObject* OwnerOuter = GetOuter();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Cast<AActor>(OwnerOuter); // 풀 컴포넌트를 가진 액터를 소유자로
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AProBulletBase* NewBullet = GetWorld()->SpawnActor<AProBulletBase>(BulletClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
		{
			// 풀에 추가
			BulletPool.Add(NewBullet);

			// Bullet에게 자기 풀을 알려주고 싶다면
			// NewBullet->SetPool(this);

			return NewBullet;
		}
	}
	return nullptr;
}

void UBulletItemInstance::ReleaseBullet(AProBulletBase* Bullet)
{
	if (!Bullet) return;
    
	// Deactivate 호출 (Bullet 자체 로직)
	Bullet->DeactivateBullet();

	// Bullet은 여전히 BulletPool에 포함됨 (즉, 재활성화될 때까지 대기)
	// 여기서 추가적으로 설정할 것이 있으면 가능 (ex: 타이머 리셋 등)
}
