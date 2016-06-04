// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommandDispatcher.h"

/**
 * Define a set of commands to interact with the Unreal World
 */
class UNREALCV_API UE4CVCommands
{
private:
	/** Store a Character pointer, so a command can control the character */
	APawn* Character;
	/** Store a FCommandDispatcher pointer, so a command can invoke other commands */
	FCommandDispatcher* CommandDispatcher;
	/** Register commands to a CommandDispatcher */
	void RegisterCommands();
	// See more details in RegisterCommands()

public:
	UE4CVCommands(APawn* MyCharacter, FCommandDispatcher* InCommandDispatcher);
	~UE4CVCommands();
};
