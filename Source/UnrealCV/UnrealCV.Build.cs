// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
using System.IO;

namespace UnrealBuildTool.Rules
{
	public class UnrealCV: ModuleRules
	{
		public UnrealCV(TargetInfo Target)
		{
			// This trick is from https://answers.unrealengine.com/questions/258689/how-to-include-private-header-files-of-other-modul.html
			string EnginePath = Path.GetFullPath(BuildConfiguration.RelativeEnginePath);

			PublicIncludePaths.AddRange(
				new string[]
				{
					EnginePath + "Source/Runtime/Launch/Resources",
					// To get Unreal Engine minor version
				}
			);

			PrivateIncludePaths.AddRange(
				new string[] {
					"UnrealCV/Private/Commands",
					"UnrealCV/Private/libs", // For 3rd-party libs
				}
			);

			PublicDependencyModuleNames.AddRange(new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"RenderCore",
				"Networking",
				"Sockets",
				"Slate",
				"ImageWrapper",
				"CinematicCamera",
				"Projects", // Support IPluginManager
			});

			// PrivateDependency only available in Private folder
			// Reference: https://answers.unrealengine.com/questions/23384/what-is-the-difference-between-publicdependencymod.html
			if (UEBuildConfiguration.bBuildEditor == true)
			{
				PrivateDependencyModuleNames.AddRange(
					new string[]
					{
						"UnrealEd", // To support GetGameWorld
					}
				);
			}

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					"Renderer"
				}
			);
		}
	}
}
