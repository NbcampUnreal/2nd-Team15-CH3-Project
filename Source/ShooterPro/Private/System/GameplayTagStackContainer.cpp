// Fill out your copyright notice in the Description page of Project Settings.


#include "System/GameplayTagStackContainer.h"
#include "GameplayTagContainer.h"

void FGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid()) return;

	if (StackCount == 0) return;
	
	if (TagMap.Contains(Tag)) TagMap[Tag] += StackCount;
	else TagMap.Add(Tag, StackCount);
}

void FGameplayTagStackContainer::RemoveStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid()) return;

	if (StackCount == 0) return;

	if (!TagMap.Contains(Tag)) return;

	if (TagMap[Tag] <= StackCount) TagMap.Remove(Tag);
	else TagMap[Tag] -= StackCount;
}
