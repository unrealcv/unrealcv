#pragma once
#include "CommandDispatcher.h"
#include "CommandHandler.h"

class FPluginHandler : public FCommandHandler
{
public:
	void RegisterCommands();

	FExecStatus GetPort(const TArray<FString>& Args);
	FExecStatus SetPort(const TArray<FString>& Args);
	FExecStatus GetUnrealCVStatus(const TArray<FString>& Args);
	/** Get the help message of defined commands */
	FExecStatus GetCommands(const TArray<FString>& Args);
	FExecStatus Echo(const TArray<FString>& Args);
	/** vget /unrealcv/version */
	FExecStatus GetVersion(const TArray<FString>& Args);

	/** vget /scene/name */
	FExecStatus GetSceneName(const TArray<FString>& Args);

	FExecStatus GetLevelName(const TArray<FString>& Args);
};
