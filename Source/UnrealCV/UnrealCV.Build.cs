// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
using System.IO;
using System.Collections.Generic;

/// <summary>
/// Engine-version-independent build configuration for the UnrealCV plugin.
/// </summary>
public class UnrealcvBuildConfig
{
	public List<string> PrivateIncludePaths = new List<string>();
	public List<string> PublicIncludePaths = new List<string>();
	public List<string> PublicDependencyModuleNames = new List<string>();
	public List<string> EditorPrivateDependencyModuleNames = new List<string>();
	public List<string> DynamicallyLoadedModuleNames = new List<string>();

	public UnrealcvBuildConfig(string EnginePath)
	{
		PrivateIncludePaths.AddRange(new string[]
		{
			"UnrealCV/Private",
			"UnrealCV/Private/Actor",
			"UnrealCV/Public/Actor",
			"UnrealCV/Public/BPFunctionLib",
			"UnrealCV/Public/Component",
			"UnrealCV/Public/Controller",
			"UnrealCV/Public/Sensor",
			"UnrealCV/Public/Sensor/CameraSensor",
			"UnrealCV/Public/Server",
			"UnrealCV/Public/Utils",
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
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
			"Projects",
			"RHI",
			"Json",
		});

		EditorPrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
		});

		DynamicallyLoadedModuleNames.AddRange(new string[]
		{
			"Renderer",
		});
	}
}

namespace UnrealBuildTool.Rules
{
	public class UnrealCV : ModuleRules
	{
		public UnrealCV(ReadOnlyTargetRules Target) : base(Target)
		{
			PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
			bEnforceIWYU = true;

			string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);
			var BuildConfig = new UnrealcvBuildConfig(EnginePath);

			PublicIncludePaths = BuildConfig.PublicIncludePaths;
			PrivateIncludePaths = BuildConfig.PrivateIncludePaths;
			PublicDependencyModuleNames = BuildConfig.PublicDependencyModuleNames;
			DynamicallyLoadedModuleNames = BuildConfig.DynamicallyLoadedModuleNames;

			if (Target.bBuildEditor)
			{
				PrivateDependencyModuleNames = BuildConfig.EditorPrivateDependencyModuleNames;
			}
		}
	}
}
