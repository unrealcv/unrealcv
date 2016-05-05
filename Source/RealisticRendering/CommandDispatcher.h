// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
class REALISTICRENDERING_API FCommandDispatcher
{
public:
	FCommandDispatcher();
	~FCommandDispatcher();
	bool BindCommand(const FString Uri, const FConsoleCommandWithArgsDelegate& Command); // Parse URI
	bool Exec(const FString Uri);
};
