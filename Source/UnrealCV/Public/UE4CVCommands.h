// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommandDispatcher.h"
#include "CommandHandler.h"

/**
 * Define a set of commands to interact with the Unreal World
 */
class UNREALCV_API UE4CVCommands
{
private:
	/** Store a FCommandDispatcher pointer, so a command can invoke other commands */
	FCommandDispatcher* CommandDispatcher;
	/** Register commands to a CommandDispatcher */
	void RegisterCommands();
	// See more details in RegisterCommands()
	TArray<FCommandHandler*> CommandHandlers;

public:
	UE4CVCommands(FCommandDispatcher* InCommandDispatcher);
	~UE4CVCommands();
};
