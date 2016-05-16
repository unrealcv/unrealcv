// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 
 */
#include "CommandDispatcher.h"
#include "NetworkManager.h"


class FRequest
{
public:
	FRequest()
	{
	}
	FRequest(FString InMessage, uint32 InRequestId) : Message(InMessage), RequestId(InRequestId)
	{
	}
	FString Message;
	uint32 RequestId;
};

class REALISTICRENDERING_API FUE4CVServer
{
public:
	FUE4CVServer(FCommandDispatcher* CommandDispatcher);
	~FUE4CVServer();
	bool Start();
	void SendClientMessage(FString Message);
	void ProcessPendingRequest();

	// Expose this for UI interaction
	UNetworkManager* NetworkManager;
private:
	TQueue<FRequest, EQueueMode::Spsc> PendingRequest; // TQueue is a thread safe implementation
	FCommandDispatcher* CommandDispatcher;
	void HandleRequest(const FString& RawMessage);
};
