#pragma once
#include "CommandDispatcher.h"
#include "UE4CVServer.h"

class FCommandHandler
{
public:
	FCommandHandler(FCommandDispatcher* InCommandDispatcher)
	: CommandDispatcher(InCommandDispatcher)
	{}
	virtual void RegisterCommands() {}; // will be overrided
	virtual ~FCommandHandler() {};
	UWorld* GetWorld()
	{
		return FUE4CVServer::Get().GetGameWorld();
	}
protected:
	FCommandDispatcher* CommandDispatcher;
};
