// Fill out your copyright notice in the Description page of Project Settings.


#include "Equipment/Abilities/ProGameplayAbility_RangedWeapon.h"

#include "Equipment/EquipmentInstance.h"
#include "Equipment/Weapon/RangedWeaponInstance.h"
#include "Projectile/ProjectileManagerComponent.h"

void UProGameplayAbility_RangedWeapon::FireWeapon(FVector StartLocation, FVector Direction)
{
	URangedWeaponInstance* RangedWeaponInstance = GetSourceRangedWeaponInstance();

	UProjectileManagerComponent* ProjectileManager = Cast<UProjectileManagerComponent>(GetAvatarActorFromActorInfo()->GetComponentByClass(UProjectileManagerComponent::StaticClass()));

	if (!ensure(ProjectileManager)) return;
	
	int BulletPerCatridge = RangedWeaponInstance->GetBulletsPerCartridge();

	for (int i = 0; i < BulletPerCatridge; ++i)
	{
		//ProjectileManager->RequestBullet();
	}
}

FHitResult UProGameplayAbility_RangedWeapon::GetHitResultWithRayCast()
{	
	APlayerController* PlayerController = Cast<APlayerController>(GetAvatarActorFromActorInfo()->GetComponentByClass(APlayerController::StaticClass()));
	
	int32 ViewportX = 0, ViewportY = 0;

	PlayerController->GetViewportSize(ViewportX, ViewportY);

	FVector2D ScreenCenter(ViewportX * 0.5f, ViewportY * 0.5f);

	FVector WorldLocation, WorldDirection;

	PlayerController->DeprojectScreenPositionToWorld(ScreenCenter.X, ScreenCenter.Y, WorldLocation, WorldDirection);

	URangedWeaponInstance* RangedWeaponInstance = GetSourceRangedWeaponInstance();
	RangedWeaponInstance->GetMaxDamageRange();
	
	FVector TraceStart = WorldLocation;
	//FVector TraceEnd = TraceStart + (WorldDirection * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(PerformCameraCenterTrace), true);

	//PlayerController->GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, TraceParams);

	return HitResult;
}


URangedWeaponInstance* UProGameplayAbility_RangedWeapon::GetSourceRangedWeaponInstance() const
{
	if (UEquipmentInstance* Equipment = GetSourceEquipmentInstance())
	{
		return Cast<URangedWeaponInstance>(Equipment);
	}
	return nullptr;
}
