// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProGameplayAbility_RangedWeapon.h"

#include "Equipment/EquipmentInstance.h"
#include "Equipment/Weapon/RangedWeaponInstance.h"
#include "Inventory/ProBulletBase.h"
#include "Projectile/ProjectileManagerComponent.h"

void UProGameplayAbility_RangedWeapon::FireWeapon(FVector StartLocation, FVector Direction, TSubclassOf<AProBulletBase> BulletClass)
{
	URangedWeaponInstance* WeaponInstance = GetSourceRangedWeaponInstance();

	UProjectileManagerComponent* ProjectileManager = Cast<UProjectileManagerComponent>(GetAvatarActorFromActorInfo()->GetComponentByClass(UProjectileManagerComponent::StaticClass()));

	if (!ensure(ProjectileManager)) return;
	
	int BulletPerCartridge = WeaponInstance->GetBulletsPerCartridge();

	for (int i = 0; i < BulletPerCartridge; ++i)
	{
		const float SpreadAngle = WeaponInstance->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = WeaponInstance->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = SpreadAngle * SpreadAngleMultiplier;

		const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

		const FVector ActualDir = RandConeNormalDistribution(Direction, HalfSpreadAngleInRadians, WeaponInstance->GetSpreadExponent());
		
		AProBulletBase* Bullet = ProjectileManager->RequestBullet(BulletClass);
		FRotator BulletRotation = ActualDir.Rotation();
		Bullet->ActivateBullet(GetAvatarActorFromActorInfo(), StartLocation, BulletRotation, ActualDir);
	}
}


FVector UProGameplayAbility_RangedWeapon::RandConeNormalDistribution(const FVector& Dir, const float ConeHalfDegree,
	const float Exponent)
{
	if (ConeHalfDegree > 0.f)
	{
		// 0~1 사이 난수 생성
		const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
		// 원뿔 중심에서 벗어나는 각도 생성
		const float AngleFromCenter = FromCenter * ConeHalfDegree;
		// 0~360도 범위에 랜덤 회전 각도 생성
		const float AngleAround = FMath::FRand() * 360.f;

		// 현재 조준 방향을 회전 값으로 변환
		FRotator Rot = Dir.Rotation();
		// 계산할거니 쿼터니언으로 변환
		FQuat DirQuat(Rot);

		// 원뿔 중심에서 벗어나는 방향을 쿼터니언 회전으로 적용
		FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));

		//원뿔 중심으로 랜덤 회전 적용
		FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));

		//최종 회전 : DirQuat(조준경 방향) * AroundQuat(원형 회전) * FromCenterQuat(확산 적용)
		FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
		FinalDirectionQuat.Normalize();

		return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
	}
	else
	{
		return Dir.GetSafeNormal();
	}
}


FVector UProGameplayAbility_RangedWeapon::GetHitResultWithRayCast(APlayerController* Controller)
{	
	int32 ViewportX = 0, ViewportY = 0;

	Controller->GetViewportSize(ViewportX, ViewportY);

	FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

	FVector WorldLocation, WorldDirection;

	Controller->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection);

	URangedWeaponInstance* RangedWeaponInstance = GetSourceRangedWeaponInstance();
	
	FVector TraceStart = WorldLocation;
	FVector TraceEnd = TraceStart + (WorldDirection * RangedWeaponInstance->GetLineTraceRange());

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(PerformCameraCenterTrace), true);

	if (Controller->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams))
	{
		return HitResult.ImpactPoint;
	}

	return TraceEnd;
}


URangedWeaponInstance* UProGameplayAbility_RangedWeapon::GetSourceRangedWeaponInstance() const
{
	if (UEquipmentInstance* Equipment = GetSourceEquipmentInstance())
	{
		return Cast<URangedWeaponInstance>(Equipment);
	}
	return nullptr;
}
