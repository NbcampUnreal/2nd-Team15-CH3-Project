#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"
#include "UObject/Object.h"


namespace ProGameplayTags
{
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Melee);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Melee2);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Melee3);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Range);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Jump);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Equip);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_UnEquip);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Reload);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_WeaponFire);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Hit);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_HitReact);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Death);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_ADS);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Regen_Health);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Regen_Mana);

	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CoolDown_Ability_Melee);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CoolDown_Ability_Melee2);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CoolDown_Ability_Melee3);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(CoolDown_Ability_Range);

	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact_Zombie);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_PoisonEffect);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Projectile_HitEffect);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Weapon_Fire);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_BurnEffect);


	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEffect_Poison);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayEffect_Burn);

	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Poison);


	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_QuickBar_SlotsAddedChanged);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_QuickBar_SlotsRemovedChanged);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_QuickBar_ActiveIndexChanged);

	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_TraceCollision);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_MotionWarping);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_SpawnBullet);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Throw);


	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_System_ShowWidget);


	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(SetByCaller_Damage);

	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_SpareAmmo);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_MagazineSize);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Weapon_MagazineAmmo)


	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Zombie_Melee);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Zombie_Range);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Enemy_Zombie_Special);


	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Burning);


	SHOOTERPRO_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);


	// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
	SHOOTERPRO_API extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	SHOOTERPRO_API extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;
};
