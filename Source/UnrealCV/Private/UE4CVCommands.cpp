// Fill out your copyright notice in the Description page of Project Settings.

// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "UE4CVCommands.h"
#include "ViewMode.h"
#include "ObjectPainter.h"
#include "CommandHandler.h"

UE4CVCommands::UE4CVCommands(FCommandDispatcher* InCommandDispatcher)
{
	this->CommandDispatcher = InCommandDispatcher;
	this->RegisterCommands();
}

UE4CVCommands::~UE4CVCommands()
{
	for (FCommandHandler* Handler : CommandHandlers)
	{
		delete Handler;
	}
}

void UE4CVCommands::RegisterCommands()
{
	if (CommandHandlers.Num() != 0) return;
	// Need to keep the reference
	CommandHandlers.Add(new FObjectCommandHandler(CommandDispatcher));
	CommandHandlers.Add(new FCameraCommandHandler(CommandDispatcher));
	CommandHandlers.Add(new FPluginCommandHandler(CommandDispatcher));

	for (FCommandHandler* Handler : CommandHandlers)
	{
		Handler->RegisterCommands();
	}

	// CommandDispatcher->BindCommand("vset /mode/(?<ViewMode>.*)", SetViewMode); // Better to check the correctness at compile time
	// The regular expression for float number is from here, http://stackoverflow.com/questions/12643009/regular-expression-for-floating-point-numbers
	// Use ICU regexp to define URI, See http://userguide.icu-project.org/strings/regexp

	CommandDispatcher->Alias("ls", "vget /util/get_commands", "List all commands");
}
