// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShooterPro : ModuleRules
{
	public ShooterPro(ReadOnlyTargetRules Target) : base(Target)
	{
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" ,"AIModule","NavigationSystem",
			"FunctionalTesting", "GameplayTags", "ModularGameplay"
		});
	}
}