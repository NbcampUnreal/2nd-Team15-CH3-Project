// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagStackContainer.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct SHOOTERPRO_API FGameplayTagStackContainer
{
	GENERATED_BODY()
	FGameplayTagStackContainer() {}

	void AddStack(FGameplayTag Tag, int32 StackCount);

	void RemoveStack(FGameplayTag Tag, int32 StackCount);

	int32 GetStackCount(FGameplayTag Tag) const
	{
		if (!TagMap.Contains(Tag)) return 0;
		return TagMap.FindRef(Tag);
	}

	bool ContainsTag(FGameplayTag Tag) const
	{
		return TagMap.Contains(Tag);
	}
private:
	TMap<FGameplayTag, int32> TagMap;
};
