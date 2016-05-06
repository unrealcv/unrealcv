// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Regex.h"


/**
 * 
 */
class FExecStatus
{
public:
	FString Message;
	static FExecStatus OK;
	static FExecStatus InvalidArgument;
	static FExecStatus Error(FString ErrorMessage);
	FExecStatus(FString Message);
	~FExecStatus();
};
DECLARE_DELEGATE_RetVal_OneParam(FExecStatus, FDispatcherDelegate, const TArray< FString >&);

class REALISTICRENDERING_API FCommandDispatcher
{
public:
	FCommandDispatcher();
	~FCommandDispatcher();
	bool BindCommand(const FString UriTemplate, const FDispatcherDelegate& Command); // Parse URI
	// bool BindCommand(const FString Uri, const FConsoleCommandDelegate& Command); // Parse URI
	FExecStatus Exec(const FString Uri);
	bool Alias(const FString Alias, const FString Command);
	bool Alias(const FString Alias, const TArray<FString>& Commands);

private:
	bool Match(const FString Uri);
	TMap<FString, FDispatcherDelegate> UriMapping;
	uint32 NumArgsLimit = 32;

	void UEConsoleGetHelper(const TArray<FString>& Args);
	void UEConsoleSetHelper(const TArray<FString>& Args);
};
