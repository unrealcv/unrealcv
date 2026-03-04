// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
// Provide abstraction for different engine versions.
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Core/Public/Misc/App.h"

/** Return the project name for the currently running application. */
inline FString GetProjectName()
{
	return FApp::GetProjectName();
}
