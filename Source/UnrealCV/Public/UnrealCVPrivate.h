#pragma once
/*
#include "CoreUObject.h"

// You should place include statements to your module's private header files here.  You only need to
// add includes for headers that are used in most of your module's source files though.
#include "IUObjectPlugin.h"
*/
// Precompiled header file for UnrealCV
#include "Engine.h"

DECLARE_STATS_GROUP(TEXT("UnrealCV"), STATGROUP_UnrealCV, STATCAT_Advanced);

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCV, Log, All);

DECLARE_CYCLE_STAT(TEXT("SaveExr"), STAT_SaveExr, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("SavePng"), STAT_SavePng, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("SaveFile"), STAT_SaveFile, STATGROUP_UnrealCV);
// DECLARE_CYCLE_STAT(TEXT("ReadPixels"), STAT_ReadPixels, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("ImageWrapper"), STAT_ImageWrapper, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("GetResource"), STAT_GetResource, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("ReadPixels from SceneCapture2D"), STAT_ReadPixels, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("CaptureScene of SceneCapture2D"), STAT_CaptureScene, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("ReadSurfaceData in the rendering thread"), STAT_ReadSurfaceData, STATGROUP_UnrealCV);
