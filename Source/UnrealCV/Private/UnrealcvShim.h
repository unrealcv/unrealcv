// Weichao Qiu @ 2018
// Provide abstraction for different engine versions
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Runtime/Core/Public/Misc/App.h"

inline FString GetProjectName()
{
#if ENGINE_MINOR_VERSION >= 18  // Assume major version is 4
	FString SceneName = FApp::GetProjectName();
#else
	FString SceneName = FApp::GetGameName();
	// This is marked as deprecated in 4.18
	// https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Source/Runtime/Core/Public/Misc/App.h:L91
#endif
	return SceneName;
}