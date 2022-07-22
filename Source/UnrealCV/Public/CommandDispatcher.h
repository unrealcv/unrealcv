// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Internationalization/Regex.h"
#include "ExecStatus.h"

// DECLARE_DELEGATE(FCallbackDelegate);
DECLARE_DELEGATE_OneParam(FCallbackDelegate, FExecStatus); // Callback needs to be set before Exec, accept ExecStatus
DECLARE_DELEGATE_RetVal_OneParam(FExecStatus, FDispatcherDelegate, const TArray< FString >&);

/**
 * Engine to execute commands
 */
class UNREALCV_API FCommandDispatcher
{
public:
	FCommandDispatcher();
	~FCommandDispatcher();
	bool BindCommand(const FString& UriTemplate, const FDispatcherDelegate& Command, const FString& Description); // Parse URI
	bool Alias(const FString& Alias, const FString& Command, const FString& Description);
	bool Alias(const FString& Alias, const TArray<FString>& Commands, const FString& Description);
	// bool BindCommand(const FString Uri, const FConsoleCommandDelegate& Command); // Parse URI
	FExecStatus Exec(const FString Uri);
	void ExecAsync(const FString Uri, const FCallbackDelegate Callback);

	/** Command handler for vrun */
	FExecStatus AliasHelper(const TArray<FString>& Args);
	/** Return help message for each command */
	const TMap<FString, FString>& GetUriDescription();

private:
	/** Store which URI handler */
	TMap<FString, FDispatcherDelegate> UriMapping;

	/** The complete list of binded URI, the order of adding is preserved */
	TArray<FString> UriList;

	/** Store help message */
	TMap<FString, FString> UriDescription; // Contains help message

	/** Store the definition of an alias */
	TMap<FString, TArray<FString> > AliasMapping;

	/** Store the regular expression for each type */
	TMap<FString, FString> TypeRegexp; // Define regular expression for type

	uint32 NumArgsLimit = 32;

	/** Convert from a human readable URI to a regular expression */
	bool FormatUri(const FString& RawUri, FString& UriRexexp);
};
