// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "TcpServer.h"
#include "Tickable.h"
#include "ServerConfig.h"

class FCommandDispatcher;
class FCommandHandler;

class FAsyncRecord
{
public:
	bool bIsCompleted;
	static TArray<FAsyncRecord*> Records; // Tracking all async task in the system
	static FAsyncRecord* Create()
	{
		FAsyncRecord* Record = new FAsyncRecord();
		return Record;
	}
	void Destory()
	{
		delete this;
	}
};

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

	/** Send a string message to connected clients */
	void SendClientMessage(FString Message);


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
		return bIsTicking; // Need to keep processing commands when game is paused
	}

	virtual TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT( FUE4CVServer, STATGROUP_Tickables );
	}

	void RegisterCommandHandlers();

	/** Make sure UE4CVServer correctly initialized itself in the GameWorld */
	bool InitWorld();

	/** Return the GameWorld of the editor or of the game */
	UWorld* GetGameWorld();

	/** Update input mode */
	void UpdateInput(bool Enable);

	/** Open new level */
	void OpenLevel(FName LevelName);

	/** The config of UE4CVServer */
	FServerConfig Config;

	/** The underlying class to handle network connection, ip and port are configured here */
	UNetworkManager* NetworkManager;
private:
	/** Handlers for UnrealCV commands */
	TArray<FCommandHandler*> CommandHandlers;

	/** Process pending requests in a tick */
	void ProcessPendingRequest();

	/** The Pawn of the Game */
	APawn* Pawn;

	bool bIsTicking = true;

	/** Construct a server */
	FUE4CVServer();

	/** Store pending requests, A new request will be stored here and be processed in the next tick of GameThread */
	TQueue<FRequest, EQueueMode::Spsc> PendingRequest; // TQueue is a thread safe implementation

	/** Handle the raw message from NetworkManager and parse raw message to a FRequest */
	void HandleRawMessage(const FString& RawMessage);

	/** Handle errors from NetworkManager */
	void HandleError(const FString& ErrorMessage);

};
