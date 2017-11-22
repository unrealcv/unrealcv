#pragma once
/*
#include "CoreUObject.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.
#include "IUObjectPlugin.h"
*/
// Precompiled header file for UnrealCV
#include "Engine.h"
#include "Version.h"

DECLARE_STATS_GROUP(TEXT("UnrealCV"), STATGROUP_UnrealCV, STATCAT_Advanced);

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCV, Log, All);

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
