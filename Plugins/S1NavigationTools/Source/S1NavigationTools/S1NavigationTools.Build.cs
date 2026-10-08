using UnrealBuildTool;

public class S1NavigationTools : ModuleRules
{
	public S1NavigationTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"S1",
			"ToolMenus",
			"Slate",
			"SlateCore",
			"NavigationSystem"
		});
	}
}
