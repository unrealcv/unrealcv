// Weichao Qiu @ 2016
#include "UnrealcvServer.h"
#include "Runtime/Engine/Classes/Engine/GameEngine.h"
//#include "Runtime/Core/Public/Internationalization/Regex.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#endif

#include "ConsoleHelper.h"
#include "Commands/ObjectHandler.h"
#include "Commands/PluginHandler.h"
#include "Commands/ActionHandler.h"
#include "Commands/AliasHandler.h"
#include "Commands/CameraHandler.h"
#include "WorldController.h"
#include "UnrealcvLog.h"
#include "UnrealcvStats.h"

DECLARE_CYCLE_STAT(TEXT("FUnrealcvServer::Tick"), STAT_Tick, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("FUnrealcvServer::ProcessRequest"), STAT_ProcessRequest, STATGROUP_UnrealCV);


void FUnrealcvServer::Tick(float DeltaTime)
{
	// Spawn a AUnrealcvWorldController, which is responsible for modifying the world to add UnrealCV functions.
	// TODO: Check whether stopping the game will reset this ptr?
	SCOPE_CYCLE_COUNTER(STAT_Tick);
	InitWorldController();
	ProcessPendingRequest();
}

void FUnrealcvServer::InitWorldController()
{
	UWorld* GameWorld = GetGameWorld();
	if (IsValid(GameWorld) && !WorldController.IsValid())
	{
		UE_LOG(LogTemp, Display, TEXT("FUnrealcvServer::Tick Create WorldController"));
		this->WorldController = Cast<AUnrealcvWorldController>(GameWorld->SpawnActor(AUnrealcvWorldController::StaticClass()));
		// if (IsValid(this->WorldController))
		if (this->WorldController != nullptr)
		{
			this->WorldController->InitWorld();
		}
		else
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Failed to spawn WorldController"));
		}
		// Its BeginPlay event will extend the GameWorld
	}
}

/** Only available during game play */
APawn* FUnrealcvServer::GetPawn()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();

	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	Pawn = PlayerController->GetPawn();
	return Pawn;
}

FUnrealcvServer& FUnrealcvServer::Get()
{
	static FUnrealcvServer Singleton;
	return Singleton;
}

void FUnrealcvServer::RegisterCommandHandlers()
{
	// Taken from ctor, because might cause loop-invoke.
	CommandHandlers.Add(new FObjectHandler());
	CommandHandlers.Add(new FPluginHandler());
	CommandHandlers.Add(new FActionHandler());
	CommandHandlers.Add(new FAliasHandler());
	CommandHandlers.Add(new FCameraHandler());
	for (FCommandHandler* Handler : CommandHandlers)
	{
		Handler->CommandDispatcher = CommandDispatcher;
		Handler->RegisterCommands();
	}
}

FUnrealcvServer::FUnrealcvServer() : myRegexPattern(MessageFormat)
{
	// Init Server
	// Code defined here should not use FUnrealcvServer::Get();
	//TcpServer = NewObject<UTcpServer>();
	TcpServer = NewObject<UUnixTcpServer>();
	CommandDispatcher = TSharedPtr<FCommandDispatcher>(new FCommandDispatcher());
	FConsoleHelper::Get().SetCommandDispatcher(CommandDispatcher);
	TcpServer->AddToRoot(); // Avoid GC
	TcpServer->UnrealcvServer = this;
	//TcpServer->OnReceived().AddRaw(this, &FUnrealcvServer::HandleRawMessage); // not bind at this stage, move the binding to the UUnixTcpServer
	//TcpServer->CommandDispatcher = CommandDispatcher;
	TcpServer->OnError().AddRaw(this, &FUnrealcvServer::HandleError);
}

FUnrealcvServer::~FUnrealcvServer()
{
	// this->TcpServer->FinishDestroy(); // TODO: Check is this usage correct?
}

// TODO: Write an article to explain this.
/** Select and return and most suitable world for current condition
 * GWorld returns the EditorWorld in the Editor, which is usually not what we need. 
 */
UWorld* FUnrealcvServer::GetWorld()
{
	UWorld* WorldPtr = nullptr;
#if WITH_EDITOR
	UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
	if (IsValid(EditorEngine))
	{
		if (EditorEngine->GetPIEWorldContext() != nullptr)
		{
			WorldPtr = EditorEngine->GetPIEWorldContext()->World();
		}
		else
		{
			WorldPtr = EditorEngine->GetEditorWorldContext().World();
		}
	} // else standalone game mode in editor
#endif

	if (!IsValid(WorldPtr))
	{
		UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
		if (IsValid(GameEngine))
		{
			WorldPtr = GameEngine->GetGameWorld(); // Not GetWorld !
		}
		else
		{
			UE_LOG(LogUnrealCV, Error, TEXT("GameEngine is invalid"));
		}

	}

	if (IsValid(WorldPtr))
	{
		return WorldPtr;
	}
	else
	{
		UE_LOG(LogUnrealCV, Error, TEXT("UWorld pointer is invalid: %p."), WorldPtr);
		return nullptr;
	}
}

/** Should be avoided */
UWorld* FUnrealcvServer::GetGameWorld()
{
	UWorld* World = nullptr;
	// The correct way to get GameWorld;
#if WITH_EDITOR
	UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine);
	if (EditorEngine != nullptr)
	{
		World = EditorEngine->PlayWorld;
		if (IsValid(World) && World->IsGameWorld())
		{
			return World;
		}
		else
		{
			// UE_LOG(LogUnrealCV, Error, TEXT("Can not get PlayWorld from EditorEngine"));
			return nullptr;
		}
	}
#endif

	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine != nullptr)
	{
		World = GameEngine->GetGameWorld();
		if (IsValid(World))
		{
			return World;
		}
		else
		{
			// UE_LOG(LogUnrealCV, Error, TEXT("Can not get GameWorld from GameEngine"));
			return nullptr;
		}
	}

	return nullptr;
}


/**
 * Make sure the UnrealcvServer is correctly configured.
 */
/* TODO: Make sure the BeginPlay or UnrealcvWorldController did exactlly the same thing
bool FUnrealcvServer::InitWorld()
{
	UWorld *World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}
	// Use this to replace BeginPlay()
	static UWorld *CurrentWorld = nullptr;
	if (CurrentWorld != World)
	{
		// Invoke this everytime when the World changes
		// This will happen when the game is stopped and restart in the UE4Editor
		APlayerController* PlayerController = World->GetFirstPlayerController();
		check(PlayerController);

		// Update camera FOV
		PlayerController->PlayerCameraManager->SetFOV(Config.FOV);

		FCaptureManager::Get().AttachGTCaptureComponentToCamera(GetPawn()); // TODO: Make this configurable in the editor

		UpdateInput(Config.EnableInput);

		FEngineShowFlags ShowFlags = World->GetGameViewport()->EngineShowFlags;
		UPlayerViewMode::Get().SaveGameDefault(ShowFlags);

		CurrentWorld = World;
	}
	return true;
}
*/

// void FUnrealcvServer::UpdateInput(bool Enable)
// {
// 	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
// 	check(PlayerController);
// 	if (Enable)
// 	{
// 		UE_LOG(LogUnrealCV, Warning, TEXT("Enabling input"));
// 		PlayerController->GetPawn()->EnableInput(PlayerController);
// 	}
// 	else
// 	{
// 		UE_LOG(LogUnrealCV, Warning, TEXT("Disabling input"));
// 		PlayerController->GetPawn()->DisableInput(PlayerController);
// 	}
// }

// void FUnrealcvServer::OpenLevel(FName LevelName)
// {
// 	UGameplayStatics::OpenLevel(GetWorld(), LevelName);
// 	UGameplayStatics::FlushLevelStreaming(GetWorld());
// 	UE_LOG(LogUnrealCV, Warning, TEXT("Level loaded"));
// }

void FUnrealcvServer::ProcessRequest(FRequest& Request)
{
	SCOPE_CYCLE_COUNTER(STAT_ProcessRequest);
	FExecStatus ExecStatus = CommandDispatcher->Exec(Request.Message); //send meesage to the command dispatcher

	// This can be removed for better performance
	//UE_LOG(LogUnrealCV, Warning, TEXT("Response: %s"), *ExecStatus.GetMessage());
	UE_LOG(LogUnrealCV, Warning, TEXT("Response id: %d"), Request.RequestId);

	FString Header = FString::Printf(TEXT("%d:"), Request.RequestId);
	TArray<uint8> ReplyData;
	FExecStatus::BinaryArrayFromString(Header, ReplyData);

	ReplyData += ExecStatus.GetData();
	TcpServer->SendData(ReplyData);
}

// Each tick of GameThread.
void FUnrealcvServer::ProcessPendingRequest()
{
	// Process all requests collected in this frame
	while (!PendingRequest.IsEmpty()) 
	{
		// if (!InitWorld()) break;

		FRequest Request;

		// Dequeue one request each time
		bool DequeueStatus = PendingRequest.Dequeue(Request);
		int32 RequestId = Request.RequestId;
		// vbatch should not stall the execution of the game thread.
		if (Request.Message.StartsWith(TEXT("vbatch"))) // vbatch should not be nested.
		{
			// Check whether it is a batch request. 
			// Run all requests until all commands are received, 
			// so that all commands can be run in the same frame
			BatchNum = FCString::Atoi(*Request.Message.Replace(TEXT("vbatch"), TEXT("")));
			if (BatchNum < 1)
			{
				UE_LOG(LogUnrealCV, Warning, TEXT("Can not handle batch smaller than 1"));
			}
			FString Header = FString::Printf(TEXT("%d:"), Request.RequestId);
			TArray<uint8> ReplyData;
			FExecStatus::BinaryArrayFromString(Header, ReplyData);
			ReplyData += FExecStatus::OK().GetData();
			TcpServer->SendData(ReplyData); // return a fake ok for vbatch
			continue;
		}
		else
		{
			BatchNum = 1;
		}

		if (BatchNum > 0) 
		// Keep collecting commands until the batch is ready
		// inside batch mode
		{
			Batch.Add(Request);
			BatchNum -= 1;
		}
		
		if (BatchNum == 0) // The batch is ready
		{
			// Otherwise hold the batch request until all commands are received.
			for (FRequest RequestToRun : Batch)
			{
				//UE_LOG(LogUnrealCV, Warning, TEXT("Run batched command %s"), *RequestToRun.Message);
				ProcessRequest(RequestToRun);
			}
			Batch.Empty();
		}
	}
}

/** Message handler for server */
void FUnrealcvServer::HandleRawMessage(const FString& Endpoint, const FString& InRawMessage)
{
	UE_LOG(LogUnrealCV, Warning, TEXT("Request: %s"), *InRawMessage);
	// Parse Raw Message
	// FString MessageFormat = "(\\d{1,}):(.*)";
	// TODO: 8 digits might not be enough if running for a very long time.
	// FRegexPattern RegexPattern(MessageFormat);
	// FRegexMatcher Matcher(RegexPattern, InRawMessage);

	FRegexMatcher Matcher(myRegexPattern, InRawMessage);

	if (Matcher.FindNext())
	{
		// TODO: Handle malform request message
		FString StrRequestId = Matcher.GetCaptureGroup(1);
		FString Message = Matcher.GetCaptureGroup(2);

		uint32 RequestId = FCString::Atoi(*StrRequestId);
		FRequest Request(Endpoint, Message, RequestId); // Create a request
		UE_LOG(LogUnrealCV, Warning, TEXT("Request: %s"), *Request.Message);
		this->PendingRequest.Enqueue(Request);
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("error: Malformat raw message '%s'"), *InRawMessage);
	}
}

/** Error handler for server */
void FUnrealcvServer::HandleError(const FString& InErrorMessage)
{
	if (Config.ExitOnFailure)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Unexpected error from server. Requesting exit. Error message: %s"), *InErrorMessage);
		FGenericPlatformMisc::RequestExit(false);
	}
}

