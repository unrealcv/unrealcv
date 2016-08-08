#pragma once
#include "CommandDispatcher.h"

class FCommandHandler
{
public:
	FCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher)
	: Character(InCharacter), CommandDispatcher(InCommandDispatcher)
	{}
	virtual void RegisterCommands() {}; // will be overrided
	virtual ~FCommandHandler() {};
protected:
	APawn* Character;
	FCommandDispatcher* CommandDispatcher;
};
