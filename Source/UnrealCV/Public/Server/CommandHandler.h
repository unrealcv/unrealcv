// Weichao Qiu @ 2016
#pragma once

#include "CommandDispatcher.h"
#include "UnrealcvServer.h"

// FExecStatus should only be used in CommandHandler
class FCommandHandler
{
public:
	virtual void RegisterCommands() = 0; 
	virtual ~FCommandHandler() {};
	UWorld* GetWorld()
	{
		return FUnrealcvServer::Get().GetWorld();
	}
	TSharedPtr<FCommandDispatcher> CommandDispatcher;
};
