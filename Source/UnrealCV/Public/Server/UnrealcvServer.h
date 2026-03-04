// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "Runtime/Engine/Public/Tickable.h"
#include "Runtime/Core/Public/Containers/Queue.h"
#include "Runtime/Core/Public/Internationalization/Regex.h"
#include "ServerConfig.h"
#include "CommandDispatcher.h"
#include "WorldController.h"
#include "UnixTcpServer.h"

/**
 * A single incoming command request from a connected client.
 */
struct FRequest
{
	FString Endpoint;
	FString Message;
	uint32  RequestId = 0;

	FRequest() = default;

	FRequest(const FString& InEndpoint, const FString& InMessage, uint32 InRequestId)
		: Endpoint(InEndpoint)
		, Message(InMessage)
		, RequestId(InRequestId)
	{
	}
};

/**
 * Central UnrealCV server — manages networking, command dispatch, and world state.
 *
 * Lifecycle:
 *  1. Start a TCP server.
 *  2. Create a FCommandDispatcher.
 *  3. Register FCommandHandler instances.
 *  4. Bind dispatcher to the TCP server and UE console.
 *  5. Each game-thread tick: process queued requests.
 */
class UNREALCV_API FUnrealcvServer : public FTickableGameObject
{
public:
	~FUnrealcvServer();

	/** Singleton accessor. */
	static FUnrealcvServer& Get();

	TSharedPtr<FCommandDispatcher> CommandDispatcher;

	/** Return the controlled Pawn (only valid during gameplay). */
	APawn* GetPawn();

	// -- FTickableGameObject interface ------------------------------------
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override            { return bIsTicking; }
	virtual bool IsTickableWhenPaused() const override   { return true; }
	virtual bool IsTickableInEditor() const override     { return true; }
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FUnrealcvServer, STATGROUP_Tickables);
	}

	void RegisterCommandHandlers();

	/** Return the best available UWorld for the current mode (editor / game). */
	UWorld* GetWorld();

	/** Return the gameplay world (nullptr outside of PIE / standalone). */
	UWorld* GetGameWorld();

	FServerConfig Config;

	UUnixTcpServer* TcpServer = nullptr;

	TWeakObjectPtr<class AUnrealcvWorldController> WorldController;

	void InitWorldController();

private:
	FUnrealcvServer();

	TArray<class FCommandHandler*> CommandHandlers;

	void ProcessPendingRequest();
	void ProcessRequest(FRequest& Request);

	int32 BatchNum = 0;
	TArray<FRequest> Batch;

	APawn* Pawn = nullptr;
	bool bIsTicking = true;

	TQueue<FRequest, EQueueMode::Spsc> PendingRequest;

	void HandleRawMessage(const FString& Endpoint, const FString& RawMessage);
	void HandleError(const FString& ErrorMessage);

	/** Regex pattern for parsing "<id>:<message>" wire format. */
	static const FString MessageFormat;
	FRegexPattern MessageRegexPattern;
};
