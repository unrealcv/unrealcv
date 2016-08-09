#pragma once
#include "CommandDispatcher.h"

class FCommandHandler
{
public:
	FCommandHandler(FCommandDispatcher* InCommandDispatcher)
	: CommandDispatcher(InCommandDispatcher)
	{}
	virtual void RegisterCommands() {}; // will be overrided
	virtual ~FCommandHandler() {};
protected:
	FCommandDispatcher* CommandDispatcher;
};
