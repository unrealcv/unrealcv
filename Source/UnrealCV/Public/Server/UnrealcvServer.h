// Weichao Qiu @ 2016
#pragma once

#include "Runtime/Engine/Public/Tickable.h"
#include "Runtime/Core/Public/Containers/Queue.h"

// #include "TcpServer.h"
#include "Runtime/Core/Public/Internationalization/Regex.h"
#include "ServerConfig.h"
#include "CommandDispatcher.h"
#include "WorldController.h"
#include "UnixTcpServer.h"

class FRequest
{
public:
	FString Endpoint;
	FString Message;
	uint32 RequestId;

	FRequest() {}
	FRequest(FString InEndpoint, FString InMessage, uint32 InRequestId) 
		: Endpoint(InEndpoint), Message(InMessage), RequestId(InRequestId) {}
};

/**
* UnrealCV server to interact with external programs.
* For UnrealCV server, when a game start:
* 1. Start a TCPserver.
* 2. Create a command dispatcher
* 3. Add command handler to command dispatcher, CameraHandler should be able to access camera
* 4. Bind command dispatcher to TCPserver
* 5. Bind command dispatcher to UE4 console
*/
class UNREALCV_API FUnrealcvServer : public FTickableGameObject
{
public:
	~FUnrealcvServer();

	/** Get the singleton */
	static FUnrealcvServer& Get();

	/** The CommandDispatcher to handle a pending request */
	TSharedPtr<class FCommandDispatcher> CommandDispatcher;

	/** Return the Pawn of this game */
	APawn* GetPawn();

	/** Implement ticking function of UnrealcvServer itself */
	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const{
		return bIsTicking;
	}

	virtual bool IsTickableWhenPaused() const
	{
		return true;
	}

	virtual bool IsTickableInEditor() const
	{
		return true;
	}

	virtual TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT( FUnrealcvServer, STATGROUP_Tickables );
	}

	void RegisterCommandHandlers();

	/** Make sure UnrealcvServer correctly initialized itself in the GameWorld */
	// bool InitWorld();

	/** Return the GameWorld of the editor or of the game */
	UWorld* GetGameWorld();

	UWorld* GetWorld();

	/** Update input mode */
	// void UpdateInput(bool Enable);

	/** Open new level */
	// void OpenLevel(FName LevelName);

	/** The config of UnrealcvServer */
	FServerConfig Config;

	/** The underlying class to handle network connection, ip and port are configured here */
	//UTcpServer* TcpServer;
	UUnixTcpServer* TcpServer;

	/** A controller to control the UE4 world */
	TWeakObjectPtr<class AUnrealcvWorldController> WorldController;

	/** InitWorldController */
	void InitWorldController();

	/** Handle the raw message from TcpServer and parse raw message to a FRequest */
	void HandleRawMessage(const FString& Endpoint, const FString& RawMessage);

private:
	/** Handlers for UnrealCV commands */
	TArray<class FCommandHandler*> CommandHandlers;

	/** Process pending requests in a tick */
	void ProcessPendingRequest();

	void ProcessRequest(FRequest& Request);

	/** The number of incoming commands for the batch mode */
	int BatchNum;

	/** Array for batch commands */
	TArray<FRequest> Batch;

	/** The Pawn of the Game */
	APawn* Pawn;

	bool bIsTicking = true;

	/** Construct a server */
	FUnrealcvServer();

	/** Store pending requests, A new request will be stored here and be processed in the next tick of GameThread */
	TQueue<FRequest, EQueueMode::Spsc> PendingRequest; // TQueue is a thread safe implementation

	/** Handle errors from TcpServer */
	void HandleError(const FString& ErrorMessage);

	FString MessageFormat = "(\\d{1,}):(.*)";  // for inf message id
	FRegexPattern myRegexPattern;

};
