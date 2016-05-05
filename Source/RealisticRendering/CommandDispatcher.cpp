// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "CommandDispatcher.h"

FCommandDispatcher::FCommandDispatcher()
{
}

FCommandDispatcher::~FCommandDispatcher()
{
}

bool FCommandDispatcher::BindCommand(const FString UriTemplate, const FConsoleCommandWithArgsDelegate& Command) // Parse URI
{
	return true;
}
	
bool FCommandDispatcher::Exec(const FString Uri)
{
	return true;
}
