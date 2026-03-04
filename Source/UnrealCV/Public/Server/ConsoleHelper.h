// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "CommandDispatcher.h"

class FConsoleOutputDevice;

/**
 * Bridges UnrealCV commands to the Unreal Engine console.
 *
 * Registers vget / vset / vrun / vexec / vbp as console commands and
 * forwards them to FCommandDispatcher for execution.
 */
class FConsoleHelper
{
public:
	static FConsoleHelper& Get();

	void SetCommandDispatcher(TSharedPtr<FCommandDispatcher> InCommandDispatcher);

	/** Obtain a console output device for the current viewport. May return null. */
	[[nodiscard]] TSharedPtr<FConsoleOutputDevice> GetConsole() const;

	// Non-copyable singleton.
	FConsoleHelper(const FConsoleHelper&) = delete;
	FConsoleHelper& operator=(const FConsoleHelper&) = delete;

private:
	FConsoleHelper();

	TSharedPtr<FCommandDispatcher> CommandDispatcher;

	/**
	 * Common implementation for all console verbs.
	 * Joins Args with the given Prefix and dispatches via CommandDispatcher.
	 */
	void DispatchConsoleCommand(const TCHAR* Prefix, const TArray<FString>& Args);

	void VGet (const TArray<FString>& Args);
	void VSet (const TArray<FString>& Args);
	void VRun (const TArray<FString>& Args);
	void VExec(const TArray<FString>& Args);
	void VBp  (const TArray<FString>& Args);
};
