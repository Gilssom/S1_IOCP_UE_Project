// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class S1 : ModuleRules
{
	public S1(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "GameplayAbilities", "GameplayTags", "GameplayTasks", "Sockets", "Networking", "UMG" });

		// Impact presentation settings are editable through Project Settings.
		PrivateDependencyModuleNames.AddRange(new string[] { "ProtobufCore", "Slate", "SlateCore", "DeveloperSettings", "Niagara" });

		PrivateIncludePaths.AddRange(new string[]
		{
			"S1/",
			"S1/Network/",
			"S1/Game/",
			"S1/Combat/",
		});


		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
