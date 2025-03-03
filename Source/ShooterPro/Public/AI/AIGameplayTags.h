// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

namespace AIGameplayTags
{
	SHOOTERPRO_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);
	
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AIState);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AIState_Idle);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AIState_Combat);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AIState_Disabled);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AIState_Investigating);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AIState_Seeking);
	SHOOTERPRO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(AIState_Dead);


	
	


	// These are mappings from MovementMode enums to GameplayTags associated with those enums (below)
	SHOOTERPRO_API extern const TMap<uint8, FGameplayTag> MovementModeTagMap;
	SHOOTERPRO_API extern const TMap<uint8, FGameplayTag> CustomMovementModeTagMap;
};
