// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TheLastShelter : ModuleRules
{
	public TheLastShelter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicIncludePaths.AddRange(new string[] {
			ModuleDirectory,
			System.IO.Path.Combine(ModuleDirectory, "Actor"),
			System.IO.Path.Combine(ModuleDirectory, "AI"),
			System.IO.Path.Combine(ModuleDirectory, "Manager"),
			System.IO.Path.Combine(ModuleDirectory, "UI")
		});

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", "CoreUObject", "Engine", "InputCore", 
			"EnhancedInput", "UMG", "PaperZD", "Paper2D",
			"Json", "JsonUtilities",
			"AIModule", "NavigationSystem",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}
