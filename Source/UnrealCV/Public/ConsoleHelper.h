// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CommandDispatcher.h"
#include "EngineUtils.h"
/**
 * Helper class to bind UnrealCV commands to Unreal Engine console
 */
class FConsoleHelper
{
public:
	// FConsoleHelper(FCommandDispatcher* CommandDispatcher);
	static FConsoleHelper& Get();
	void SetCommandDispatcher(FCommandDispatcher* CommandDispatcher);

	/** The exec result of CommandDispatcher will be written to FConsoleOutputDevice */
	FConsoleOutputDevice* GetConsole();

private:
	FConsoleHelper();

	/** The command from UE4 console will be sent to CommandDispatcher for execution */
	FCommandDispatcher* CommandDispatcher;

	/** Register vget command to UE4 console */
	void VGet(const TArray<FString>& Args);
	/** Register vset command to UE4 console */
	void VSet(const TArray<FString>& Args);
	/** Register vrun command to UE4 console */
	void VRun(const TArray<FString>& Args);
	/** Register vexec command */
	void VExec(const TArray<FString>& Args);
};
