#pragma once

#include "Engine.h"

// DECLARE_STATS_GROUP(TEXT("UnrealCV"), STATGROUP_UnrealCV, STATCAT_Advanced);

// DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCV, Log, All);

/** Utility function to print a message to the screen */
inline void ScreenLog(FString Msg)
{
	GEngine->AddOnScreenDebugMessage(-1, 60.0, FColor::Green, *Msg);
}
