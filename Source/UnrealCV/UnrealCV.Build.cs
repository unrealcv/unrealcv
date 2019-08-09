// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
using System.IO;
using System.Collections.Generic;

// An engine version independent configuration class
public class UnrealcvBuildConfig
{
	public List<string> PrivateIncludePaths = new List<string>();
	public List<string> PublicIncludePaths = new List<string>();
	public List<string> PublicDependencyModuleNames = new List<string>();
	public List<string> EditorPrivateDependencyModuleNames = new List<string>();
	public List<string> DynamicallyLoadedModuleNames = new List<string>();

	public UnrealcvBuildConfig(string EnginePath)
	{
		PublicIncludePaths.AddRange(
			new string[]
			{
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				"UnrealCV/Private",
				"UnrealCV/Private/Actor",
				"UnrealCV/Public/Actor",
				"UnrealCV/Public/BPFunctionLib",
				"UnrealCV/Public/Component",
				"UnrealCV/Public/Controller",
				"UnrealCV/Public/Sensor",
				"UnrealCV/Public/Sensor/CameraSensor",
				"UnrealCV/Public/Server",
				"UnrealCV/Public/Utils"
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
			"RHI", // Support low-level RHI operation
			"Json",
		});

		EditorPrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd", // To support GetGameWorld
				// This is only available for Editor build
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				"Renderer"
			}
		);
	}
}

namespace UnrealBuildTool.Rules
{
	public class UnrealCV: ModuleRules
	{
		// ReadOnlyTargetRules for version > 4.15
		public UnrealCV(ReadOnlyTargetRules Target) : base(Target)
		// 4.16 or better
		{
			bEnforceIWYU = true;
	  		bFasterWithoutUnity = true;
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

			// This trick is from https://answers.unrealengine.com/questions/258689/how-to-include-private-header-files-of-other-modul.html
			// string EnginePath = Path.GetFullPath(BuildConfigurationTarget.RelativeEnginePath);
			string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);
			UnrealcvBuildConfig BuildConfig = new UnrealcvBuildConfig(EnginePath);

			PublicIncludePaths = BuildConfig.PublicIncludePaths;
			PrivateIncludePaths = BuildConfig.PrivateIncludePaths;
			PublicDependencyModuleNames = BuildConfig.PublicDependencyModuleNames;
			DynamicallyLoadedModuleNames = BuildConfig.DynamicallyLoadedModuleNames;

			// PrivateDependency only available in Private folder
			// Reference: https://answers.unrealengine.com/questions/23384/what-is-the-difference-between-publicdependencymod.html
			// if (UEBuildConfiguration.bBuildEditor == true)
			if (Target.bBuildEditor == true)
			{
				PrivateDependencyModuleNames = BuildConfig.EditorPrivateDependencyModuleNames;
			}
		}
	}
}

