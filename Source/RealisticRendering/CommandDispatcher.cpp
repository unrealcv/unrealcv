// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "CommandDispatcher.h"

CommandDispatcher::CommandDispatcher()
{
}

CommandDispatcher::~CommandDispatcher()
{
}

bool CommandDispatcher::BindCommand(const FString UriTemplate, const FConsoleCommandWithArgsDelegate& Command) // Parse URI
{
	return true;
}
	
bool CommandDispatcher::Exec(const FString Uri)
{
	return true;
}
