// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
#include "CommandDispatcher.h"
#include "NetworkManager.h"
class REALISTICRENDERING_API FUE4CVServer
{
public:
	FUE4CVServer(FCommandDispatcher* CommandDispatcher, UWorld* World);
	~FUE4CVServer();
	bool Start();
	void SendClientMessage(FString Message);

	// Expose this for UI interaction
	UNetworkManager* NetworkManager;
private:
	FCommandDispatcher* CommandDispatcher;
	UWorld* World;

	void ReplyClient(uint32 RequestId, FExecStatus ExecStatus); // This is too low level, consider working on a higher level.
	void HandleClientMessage(FString RawMessage);
	void LogClientMessage(FString RawMessage);
};
