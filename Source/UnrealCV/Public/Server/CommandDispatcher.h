// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "ExecStatus.h"
#include "Runtime/Core/Public/Internationalization/Regex.h"

DECLARE_DELEGATE_OneParam(FCallbackDelegate, FExecStatus);
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

	bool BindCommand(const FString& UriTemplate, const FDispatcherDelegate& Command, const FString& Description);
	bool Alias(const FString& AliasName, const FString& Command, const FString& Description);
	bool Alias(const FString& AliasName, const TArray<FString>& Commands, const FString& Description);

	FExecStatus Exec(const FString& Uri);

	const TMap<FString, FString>& GetUriDescription() const;

private:
	FExecStatus AliasHelper(const TArray<FString>& Args);
	bool FormatUri(const FString& RawUri, FString& OutRegex) const;

	TMap<FString, FDispatcherDelegate> UriMapping;
	TArray<FString> UriList;
	TMap<FString, FRegexPattern> UriRegexPattern;
	TMap<FString, FString> UriDescription;
	TMap<FString, TArray<FString>> AliasMapping;
	TMap<FString, FString> TypeRegexp;

	static constexpr uint32 NumArgsLimit = 32;
};
