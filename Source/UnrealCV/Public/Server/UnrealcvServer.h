// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "Tickable.h"
#include "Containers/Queue.h"
#include "Internationalization/Regex.h"
#include "ServerConfig.h"
#include "CommandDispatcher.h"

class UUnixTcpServer;
class AUnrealcvWorldController;
class FCommandHandler;

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

	// Non-copyable, non-movable singleton.
	FUnrealcvServer(const FUnrealcvServer&) = delete;
	FUnrealcvServer& operator=(const FUnrealcvServer&) = delete;

	/** Singleton accessor. */
	static FUnrealcvServer& Get();

	/** Return the controlled Pawn (only valid during gameplay). */
	[[nodiscard]] APawn* GetPawn();

	// -- FTickableGameObject interface ------------------------------------
	virtual void Tick(float DeltaTime) override;
	[[nodiscard]] virtual bool IsTickable() const override            { return bIsTicking; }
	[[nodiscard]] virtual bool IsTickableWhenPaused() const override  { return true; }
	[[nodiscard]] virtual bool IsTickableInEditor() const override    { return true; }
	[[nodiscard]] virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FUnrealcvServer, STATGROUP_Tickables);
	}

	void RegisterCommandHandlers();

	/** Return the best available UWorld for the current mode (editor / game). */
	[[nodiscard]] UWorld* GetWorld() const;

	/** Return the gameplay world (nullptr outside of PIE / standalone). */
	[[nodiscard]] UWorld* GetGameWorld() const;

	[[nodiscard]] const FServerConfig& GetConfig() const { return Config; }
	FServerConfig& GetMutableConfig() { return Config; }

	[[nodiscard]] TSharedPtr<FCommandDispatcher> GetCommandDispatcher() const { return CommandDispatcher; }

	[[nodiscard]] UUnixTcpServer* GetTcpServer() const { return TcpServer; }

	[[nodiscard]] TWeakObjectPtr<AUnrealcvWorldController> GetWorldController() const { return WorldController; }

	void InitWorldController();

private:
	FUnrealcvServer();

	TWeakObjectPtr<AUnrealcvWorldController> WorldController;
	FServerConfig Config;
	TSharedPtr<FCommandDispatcher> CommandDispatcher;
	UUnixTcpServer* TcpServer = nullptr;

	TArray<TUniquePtr<FCommandHandler>> CommandHandlers;

	void ProcessPendingRequest();
	void ProcessRequest(const FRequest& Request);

	int32 BatchNum = 0;
	TArray<FRequest> Batch;

	TWeakObjectPtr<APawn> CachedPawn;
	bool bIsTicking = true;

	TQueue<FRequest, EQueueMode::Spsc> PendingRequest;

	void HandleRawMessage(const FString& Endpoint, const FString& RawMessage);
	void HandleError(const FString& ErrorMessage);

	/** Regex pattern for parsing "<id>:<message>" wire format. */
	static const FString MessageFormat;
	FRegexPattern MessageRegexPattern;
};
