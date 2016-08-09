// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

/**
 * UnrealCV server to interact with external programs
 */
class UNREALCV_API FUE4CVServer : public FTickableGameObject
{
public:
	~FUE4CVServer();

	/** Start the server */
	bool Start();

	/** Send a string message to connected clients */
	void SendClientMessage(FString Message);

	/** Process pending requests in a tick */
	void ProcessPendingRequest();

	/** The underlying class to handle network connection, ip and port are configured here */
	UNetworkManager* NetworkManager;

	/** Initialize the server with the game pawn */
	bool Init();

	/** Get the singleton */
	static FUE4CVServer& Get();

	/** The CommandDispatcher to handle a pending request */
	FCommandDispatcher* CommandDispatcher;

	/** Return the Pawn of this game */
	APawn* GetPawn();

	/** Implement ticking function of UE4CVServer itself */
	virtual void Tick(float DeltaTime) override
	{
		ProcessPendingRequest();
	}

	virtual bool IsTickable() const{
		return bIsTicking;
	}

	virtual bool IsTickableWhenPaused() const
	{
		return bIsTicking;
	}

	virtual TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT( FUE4CVServer, STATGROUP_Tickables );
	}

private:
	/** The Pawn of the Game */
	APawn* Pawn;

	bool bIsTicking = false;

	/** Construct a server */
	FUE4CVServer();

	/** Store pending requests, A new request will be stored here and be processed in the next tick of GameThread */
	TQueue<FRequest, EQueueMode::Spsc> PendingRequest; // TQueue is a thread safe implementation

	/** Handle the raw message from NetworkManager and parse raw message to a FRequest */
	void HandleRawMessage(const FString& RawMessage);
};
