#include "ProGmaeplayTag.h"


namespace ProGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ability, TEXT("Ability"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Melee, TEXT("Ability.Melee"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Melee2, TEXT("Ability.Melee2"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Melee3, TEXT("Ability.Melee3"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Range, TEXT("Ability.Range"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Hit, TEXT("Ability.Hit"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_HitReact, TEXT("Ability.HitReact"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Reload, TEXT("Ability.Reload"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Fire, TEXT("Ability.Fire"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Death, TEXT("Ability.Death"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Jump, TEXT("Ability.Jump"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_ADS, TEXT("Ability.ADS"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Regen_Health, TEXT("Ability.Regen.Health"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Regen_Mana, TEXT("Ability.Regen.Mana"));


	UE_DEFINE_GAMEPLAY_TAG(CoolDown_Ability_Melee, TEXT("CoolDown.Ability.Melee"));
	UE_DEFINE_GAMEPLAY_TAG(CoolDown_Ability_Melee2, TEXT("CoolDown.Ability.Melee2"));
	UE_DEFINE_GAMEPLAY_TAG(CoolDown_Ability_Melee3, TEXT("CoolDown.Ability.Melee3"));
	UE_DEFINE_GAMEPLAY_TAG(CoolDown_Ability_Range, TEXT("CoolDown.Ability.Range"));


	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_HitImpact_Zombie, TEXT("GameplayCue.HitImpact.Zombie"));
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_PoisonEffect, TEXT("GameplayCue.PoisonEffect"));
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_HitImpact_Pistol, TEXT("GameplayCue.HitImpact.Pistol"));
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Projectile_HitEffect, TEXT("GameplayCue.Projectile.HitImpact"));
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_Weapon_Fire, TEXT("GameplayCue.Weapon.Fire"));
	UE_DEFINE_GAMEPLAY_TAG(GameplayCue_BurnEffect, TEXT("GameplayCue.BurnEffect"));

	/// 어빌리티

	UE_DEFINE_GAMEPLAY_TAG(GameplayEffect_Poison, TEXT("GameplayEffect.Poison"));
	UE_DEFINE_GAMEPLAY_TAG(GameplayEffect_Burn, TEXT("GameplayEffect.Burn"));

	UE_DEFINE_GAMEPLAY_TAG(State_Poison, TEXT("State.Poison"));


	UE_DEFINE_GAMEPLAY_TAG(Event_QuickBar_SlotsAddedChanged, TEXT("Event.QuickBar.SlotsAddedChanged"));
	UE_DEFINE_GAMEPLAY_TAG(Event_QuickBar_SlotsRemovedChanged, TEXT("Event.QuickBar.SlotsRemovedChanged"));
	UE_DEFINE_GAMEPLAY_TAG(Event_QuickBar_ActiveIndexChanged, TEXT("Event.QuickBar.ActiveIndexChanged"));

	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_TraceCollision, TEXT("Event.Montage.TraceCollision"));
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_MotionWarping, TEXT("Event.Montage.MotionWarping"));
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_SpawnBullet, TEXT("Event.Montage.SpawnBullet"));
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Reload, TEXT("Event.Montage.Reload"));


	UE_DEFINE_GAMEPLAY_TAG(Event_System_ShowWidget, TEXT("Event.System.ShowWidget"));

	/// 무기
	UE_DEFINE_GAMEPLAY_TAG(Weapon_SpareAmmo, TEXT("Weapon.SpareAmmo"));
	UE_DEFINE_GAMEPLAY_TAG(Weapon_MagazineSize, TEXT("Weapon.MagazineSize"));
	UE_DEFINE_GAMEPLAY_TAG(Weapon_MagazineAmmo, TEXT("Weapon.MagazineAmmo"));
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Throw, TEXT("Event.Montage.Throw"));


	UE_DEFINE_GAMEPLAY_TAG(SetByCaller_Damage, TEXT("SetByCaller.Damage"));

	UE_DEFINE_GAMEPLAY_TAG(Player_State_Death, TEXT("Player.State.Death"));


	UE_DEFINE_GAMEPLAY_TAG(Enemy_Zombie_Melee, TEXT("Enemy.Zombie.Melee"));
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Zombie_Range, TEXT("Enemy.Zombie.Range"));
	UE_DEFINE_GAMEPLAY_TAG(Enemy_Zombie_Special, TEXT("Enemy.Zombie.Special"));


	UE_DEFINE_GAMEPLAY_TAG(State_Burning, TEXT("State.Burning"));


	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					// AI_ENEMY_SCREEN_LOG_LOG(5.0f, "Could not find exact match for tag [%s] but found partial match on tag [%s].", *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}


	// Unreal Movement Modes
	// const TMap<uint8, FGameplayTag> MovementModeTagMap =
	// {
	// 	{ MOVE_Walking, Movement_Mode_Walking },
	// 	{ MOVE_NavWalking, Movement_Mode_NavWalking },
	// 	{ MOVE_Falling, Movement_Mode_Falling },
	// 	{ MOVE_Swimming, Movement_Mode_Swimming },
	// 	{ MOVE_Flying, Movement_Mode_Flying },
	// 	{ MOVE_Custom, Movement_Mode_Custom }
	// };

	// Custom Movement Modes
	// const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	// {
	// 	// Fill these in with your custom modes
	// };
}
