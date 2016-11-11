#pragma once
#include "CommandHandler.h"

class FActionCommandHandler : public FCommandHandler
{
public:
	FActionCommandHandler(FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCommandDispatcher)
	{}
	void RegisterCommands();

	/** vset /action/game/pause */
	FExecStatus PauseGame(const TArray<FString>& Args);

	/** vset /action/game/resume */
	FExecStatus ResumeGame(const TArray<FString>& Args);
};
