// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Regex.h"
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
	FExecStatus AliasHelper(const TArray<FString>& Args);
	const TMap<FString, FString>& GetUriDescription();

private:
	TMap<FString, FDispatcherDelegate> UriMapping;
	TMap<FString, FString> UriDescription; // Contains help message 
	TMap<FString, TArray<FString> > AliasMapping;
	TMap<FString, FString> TypeRegexp; // Define regular expression for type
	uint32 NumArgsLimit = 32;

	bool FormatUri(const FString& RawUri, FString& UriRexexp);
};
