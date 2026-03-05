// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "CommandHandler.h"
#include "UnrealcvServer.h"

UWorld* FCommandHandler::GetWorld() const
{
	return FUnrealcvServer::Get().GetWorld();
}
