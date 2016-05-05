// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Regex.h"

/**
 * 
 */
class REALISTICRENDERING_API FCommandDispatcher
{
public:
	FCommandDispatcher();
	~FCommandDispatcher();
	bool BindCommand(const FString UriTemplate, const FConsoleCommandWithArgsDelegate& Command); // Parse URI
	// bool BindCommand(const FString Uri, const FConsoleCommandDelegate& Command); // Parse URI
	bool Exec(const FString Uri);
	bool Alias(const FString Alias, const FString Command);

private:
	bool Match(const FString Uri);
	TMap<FString, FConsoleCommandWithArgsDelegate> UriMapping;
	uint32 NumArgsLimit = 32;
};
