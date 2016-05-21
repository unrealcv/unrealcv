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
	FRequest() {}
	FRequest(FString InMessage, uint32 InRequestId) : Message(InMessage), RequestId(InRequestId) {}
	FString Message;
	uint32 RequestId;
};

class FPendingTask
{
public:
	FPendingTask(FPromise InPromise, uint32 InRequestId) : Promise(InPromise), RequestId(InRequestId), Active(true) {}
	FPendingTask() : Active(false) {}
	FExecStatus CheckStatus() { return Promise.CheckStatus();  }
	bool Active;
	FPromise Promise;
	uint32 RequestId;
};

class FUE4CVServer
{
public:
	FPendingTask PendingTask;
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
