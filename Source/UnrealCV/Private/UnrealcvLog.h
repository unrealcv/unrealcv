// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "Runtime/Core/Public/Logging/LogMacros.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCV, Log, All);

/**
 * Utility logger that suppresses duplicate messages
 * and optionally renders to the player viewport.
 */
class FUnrealcvLogger
{
public:
	/** Print a warning only once per unique message. */
	void LogOnce(const FString& LogMessage);

	/** Display a message on the player viewport. */
	void ScreenLog(const FString& LogMessage);

private:
	TSet<FString> SeenMessages;
};

/** Global logger instance (header-only singletons are fine for a plugin). */
inline FUnrealcvLogger& GetUnrealcvLogger()
{
	static FUnrealcvLogger Instance;
	return Instance;
}

#define UCV_LOG_ONCE(Msg) GetUnrealcvLogger().LogOnce(Msg)
#define UCV_SCREEN_LOG(Msg) GetUnrealcvLogger().ScreenLog(Msg)
