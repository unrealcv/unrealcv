// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class UnrealCV: ModuleRules
	{
		public UnrealCV(TargetInfo Target)
		{
            // This is tricky from https://answers.unrealengine.com/questions/258689/how-to-include-private-header-files-of-other-modul.html
            // string EnginePath = Path.GetFullPath(BuildConfiguration.RelativeEnginePath);
            // string RuntimePath = EnginePath + "Source/Runtime/";
            // string RenderPrivatePath = EnginePath + "Source/Runtime/Renderer/Private";

            /*
            PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
                    "Renderer"
				}
				);
            PublicIncludePaths.AddRange(
                new string[]
                {
                    EnginePath + "Source/Runtime/Renderer/Private",
                    EnginePath + "Source/Runtime/Renderer/Private/CompositionLighting",
                    EnginePath + "Source/Runtime/Renderer/Private/PostProcess",
                }
                );
                */

			PrivateIncludePaths.AddRange(
				new string[] {
				}
				);

            PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RenderCore", "Networking", "Sockets", "Slate", "ImageWrapper"});
            /*
			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					// ... add other public dependencies that you statically link with here ...
				}
				);
                */

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
                    // "Renderer"
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
                    "Renderer"
				}
				);
		}
	}
}