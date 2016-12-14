#pragma once
#include "CommandHandler.h"

class FAliasCommandHandler : public FCommandHandler
{
public:
	FAliasCommandHandler(FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCommandDispatcher)
	{}
	void RegisterCommands();

	/** vrun * */
	FExecStatus VRun(const TArray<FString>& Args);
};
