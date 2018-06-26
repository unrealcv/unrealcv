// Weichao Qiu @ 2017
#pragma once

#include "CommandDispatcher.h"
#include "Runtime/Engine/Public/EngineUtils.h"

/**
 * Helper class to bind UnrealCV commands to Unreal Engine console
 */
class FConsoleHelper
{
public:
	// FConsoleHelper(FCommandDispatcher* CommandDispatcher);
	static FConsoleHelper& Get();
	void SetCommandDispatcher(TSharedPtr<FCommandDispatcher> CommandDispatcher);

	/** The exec result of CommandDispatcher will be written to FConsoleOutputDevice */
	TSharedPtr<FConsoleOutputDevice> GetConsole();

private:
	FConsoleHelper();

	/** The command from UE4 console will be sent to CommandDispatcher for execution */
	TSharedPtr<FCommandDispatcher> CommandDispatcher;

	/** Register vget command to UE4 console */
	void VGet(const TArray<FString>& Args);
	/** Register vset command to UE4 console */
	void VSet(const TArray<FString>& Args);
	/** Register vrun command to UE4 console */
	void VRun(const TArray<FString>& Args);
	/** Register vexec command */
	void VExec(const TArray<FString>& Args);

	void VBp(const TArray<FString>& Args);
};
