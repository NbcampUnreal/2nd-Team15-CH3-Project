#include "ProGmaeplayTag.h"


namespace ProGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Ability, TEXT("Ability"));
	UE_DEFINE_GAMEPLAY_TAG(Ability_Melee, TEXT("Ability.Melee"));

	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_TraceCollision, TEXT("Event.Montage.TraceCollision"));
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_MotionWarping, TEXT("Event.Montage.MotionWarping"));


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
