using UnrealBuildTool;
using System.IO;

public class S1NavigationTools : ModuleRules
{
	public S1NavigationTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// Editor exporter reads the project's existing native NPC definition.
		PrivateIncludePaths.Add(Path.GetFullPath(Path.Combine(ModuleDirectory,"../../../../Source/S1")));

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"S1",
			"ToolMenus",
			"Slate",
			"SlateCore",
			"NavigationSystem",
			"Landscape",
			"PhysicsCore"
		});
	}
}
