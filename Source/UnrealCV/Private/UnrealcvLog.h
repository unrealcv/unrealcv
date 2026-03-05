// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "Runtime/Core/Public/Logging/LogMacros.h"
#include "Runtime/Core/Public/HAL/CriticalSection.h"
#include "Containers/Set.h"

DECLARE_LOG_CATEGORY_EXTERN(LogUnrealCV, Log, All);

/**
 * Utility logger that suppresses duplicate messages
 * and optionally renders to the player viewport.
 *
 * Thread-safe: SeenMessages access is guarded by a critical section.
 */
class FUnrealcvLogger
{
public:
	/** Print a warning only once per unique message. */
	void LogOnce(const FString& LogMessage);

	/** Display a message on the player viewport. */
	void ScreenLog(const FString& LogMessage);

	/** Clear the seen-messages cache (e.g. on level transitions). */
	void ResetSeenMessages();

private:
	FCriticalSection Lock;
	TSet<FString> SeenMessages;

	static constexpr int32 MaxSeenMessages = 10000;
};

/** Global logger instance (header-only singletons are fine for a plugin). */
inline FUnrealcvLogger& GetUnrealcvLogger()
{
	static FUnrealcvLogger Instance;
	return Instance;
}

#define UCV_LOG_ONCE(Msg) GetUnrealcvLogger().LogOnce(Msg)
#define UCV_SCREEN_LOG(Msg) GetUnrealcvLogger().ScreenLog(Msg)
