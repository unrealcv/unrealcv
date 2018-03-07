#pragma once
#include "CommandDispatcher.h"
#include "UE4CVServer.h"

// FExecStatus should only be used in CommandHandler
class FCommandHandler
{
public:
	virtual void RegisterCommands() = 0; 
	virtual ~FCommandHandler() {};
	UWorld* GetWorld()
	{
		return FUE4CVServer::Get().GetGameWorld();
	}
	TSharedPtr<FCommandDispatcher> CommandDispatcher;
};
