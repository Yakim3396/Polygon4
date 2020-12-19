// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class P4Tools : ModuleRules
{
	public P4Tools(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivatePCHHeaderFile = "P4ToolsPrivatePCH.h";
        MinFilesUsingPrecompiledHeaderOverride = 1;
        CppStandard = CppStandardVersion.Latest;
        bUseUnity = false;


        PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
