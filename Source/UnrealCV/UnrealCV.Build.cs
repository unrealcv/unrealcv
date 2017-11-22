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

#if WITH_FORWARDED_MODULE_RULES_CTOR
namespace UnrealBuildTool.Rules
{
	public class UnrealCV: ModuleRules
	{
		// ReadOnlyTargetRules for version > 4.15
		public UnrealCV(ReadOnlyTargetRules Target) : base(Target)
		// 4.16 or better
		{
			bEnforceIWYU = false;

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

#else //4.15 or lower, for backward compatibility
namespace UnrealBuildTool.Rules
{
	public class UnrealCV: ModuleRules
	{
		public UnrealCV(TargetInfo Target)
		{
			string EnginePath = Path.GetFullPath(BuildConfiguration.RelativeEnginePath);
			UnrealcvBuildConfig BuildConfig = new UnrealcvBuildConfig(EnginePath);

			PublicIncludePaths = BuildConfig.PublicIncludePaths;
			PrivateIncludePaths = BuildConfig.PrivateIncludePaths;
			PublicDependencyModuleNames = BuildConfig.PublicDependencyModuleNames;
			DynamicallyLoadedModuleNames = BuildConfig.DynamicallyLoadedModuleNames;

			if (UEBuildConfiguration.bBuildEditor == true)
			{
				PrivateDependencyModuleNames = BuildConfig.EditorPrivateDependencyModuleNames;
			}
		}
	}
}
#endif
