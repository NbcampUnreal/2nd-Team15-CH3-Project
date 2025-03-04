#include "AI/AIGameplayTags.h"


#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "AI/EnemyAILog.h"


namespace AIGameplayTags
{
	// UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");

	// 예: "AI.State" = EAIState
	UE_DEFINE_GAMEPLAY_TAG(AIState, TEXT("AI.State"));
	UE_DEFINE_GAMEPLAY_TAG(AIState_Idle, TEXT("AI.State.Idle"));
	UE_DEFINE_GAMEPLAY_TAG(AIState_Combat, TEXT("AI.State.Combat"));
	UE_DEFINE_GAMEPLAY_TAG(AIState_Disabled, TEXT("AI.State.Disabled"));
	UE_DEFINE_GAMEPLAY_TAG(AIState_Investigating, TEXT("AI.State.Investigating"));
	UE_DEFINE_GAMEPLAY_TAG(AIState_Seeking, TEXT("AI.State.Seeking"));
	UE_DEFINE_GAMEPLAY_TAG(AIState_Dead, TEXT("AI.State.Dead"));


	UE_DEFINE_GAMEPLAY_TAG(AI_Ability_Roar, TEXT("AI.Ability.Roar"));


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
					AI_ENEMY_SCREEN_LOG_LOG(5.0f, "Could not find exact match for tag [%s] but found partial match on tag [%s].", *TagString, *TestTag.ToString());
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
