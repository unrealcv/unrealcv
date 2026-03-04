// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "CommandDispatcher.h"
#include "UnrealcvServer.h"

/**
 * Abstract base class for command handler groups.
 *
 * Subclasses override RegisterCommands() to bind their URIs
 * to the shared CommandDispatcher.
 */
class FCommandHandler
{
public:
	virtual ~FCommandHandler() = default;
	virtual void RegisterCommands() = 0;

	UWorld* GetWorld() const
	{
		return FUnrealcvServer::Get().GetWorld();
	}

	TSharedPtr<FCommandDispatcher> CommandDispatcher;
};
