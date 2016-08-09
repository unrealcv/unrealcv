#pragma once
#include "CommandDispatcher.h"

class FPluginCommandHandler : public FCommandHandler
{
public:
	FPluginCommandHandler(FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCommandDispatcher)
	{}
	void RegisterCommands();

	FExecStatus GetPort(const TArray<FString>& Args);
	FExecStatus SetPort(const TArray<FString>& Args);
	FExecStatus GetUnrealCVStatus(const TArray<FString>& Args);
	/** Get the help message of defined commands */
	FExecStatus GetCommands(const TArray<FString>& Args);
};
