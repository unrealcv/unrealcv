// Weichao Qiu @ 2018
#pragma once
#include "Runtime/Core/Public/Logging/LogMacros.h"
DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCV, Log, All);

/** Log the message to the screen */
void ScreenLog(const FString& Message);

class FUnrealcvLogger
{
	TArray<FString> ErrorList;

public:
	/** Once print log in the console once
	 * Avoid flooding the console output */
	void LogOnce(const FString& LogMessage);

	/** Log the message to the player viewport to make it more verbose */
	void ScreenLog(const FString& LogMessage);
};

static FUnrealcvLogger GLogger;
#define LOG1(x) GLogger.LogOnce(x)
#define LOGV(x) GLogger.ScreenLog(x)
