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
			"AnimGraphRuntime"
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" ,"AIModule","NavigationSystem",
			"ModularGameplay","UMG",
			"FunctionalTesting", "GameplayTags",
			"GameplayAbilities","GameplayTasks", 
			"AIModule","GameplayBehaviorsModule",
			"MotionWarping",
			"GASCompanion", "GameplayMessageRuntime",
			"Niagara"
		});
	}
}