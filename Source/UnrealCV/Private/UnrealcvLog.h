// Weichao Qiu @ 2018
#pragma once
#include "Runtime/Core/Public/Logging/LogMacros.h"
DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCV, Log, All);
/** Log the message to the screen */
inline void ScreenLog(const FString& Message)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
}