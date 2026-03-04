// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "ExecStatus.h"
#include "Runtime/Core/Public/Internationalization/Regex.h"

DECLARE_DELEGATE_RetVal_OneParam(FExecStatus, FDispatcherDelegate, const TArray<FString>&);

/**
 * Dispatches URI-style commands to registered handlers.
 *
 * Commands are matched via regex patterns derived from human-readable
 * URI templates (e.g. "vget /camera/[uint]/lit").
 */
class UNREALCV_API FCommandDispatcher
{
public:
	FCommandDispatcher();
	~FCommandDispatcher() = default;

	/** Register a command handler for the given URI template. */
	[[nodiscard]] bool BindCommand(const FString& UriTemplate, const FDispatcherDelegate& Command, const FString& Description);

	/** Register an alias that expands to a single command. */
	[[nodiscard]] bool Alias(const FString& AliasName, const FString& Command, const FString& Description);

	/** Register an alias that expands to multiple commands. */
	[[nodiscard]] bool Alias(const FString& AliasName, const TArray<FString>& Commands, const FString& Description);

	/** Execute a command URI. Must be called on the game thread. */
	[[nodiscard]] FExecStatus Exec(const FString& Uri);

	/** Return a map of URI template -> description for all registered commands. */
	[[nodiscard]] const TMap<FString, FString>& GetUriDescription() const;

private:
	FExecStatus AliasHelper(const TArray<FString>& Args);
	[[nodiscard]] bool FormatUri(const FString& RawUri, FString& OutRegex) const;

	TMap<FString, FDispatcherDelegate> UriMapping;
	TArray<FString> UriList;
	TMap<FString, FRegexPattern> UriRegexPattern;
	TMap<FString, FString> UriDescription;
	TMap<FString, TArray<FString>> AliasMapping;
	TMap<FString, FString> TypeRegexp;

	static constexpr uint32 NumArgsLimit = 32;
};
