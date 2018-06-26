// Weichao Qiu @ 2016
#include "UnrealcvServer.h"
#include "Runtime/Engine/Classes/Engine/GameEngine.h"
#include "Controller/PlayerViewMode.h"
#include "ConsoleHelper.h"
#include "ObjectPainter.h"
#include "Commands/ObjectHandler.h"
#include "Commands/PluginHandler.h"
#include "Commands/ActionHandler.h"
#include "Commands/AliasHandler.h"
#include "Commands/SensorHandler.h"
#include "UnrealcvLog.h"

void FUnrealcvServer::Tick(float DeltaTime)
{
	// Spawn a AUnrealcvWorldController, which is responsible for modifying the world to add UnrealCV functions.
	// TODO: Check whether stopping the game will reset this ptr?
	UWorld* GameWorld = GetGameWorld();
	if (IsValid(GameWorld) && !WorldController.IsValid())
	{
		this->WorldController = Cast<AUnrealcvWorldController>(GameWorld->SpawnActor(AUnrealcvWorldController::StaticClass()));
		// Its BeginPlay event will extend the GameWorld
	}

	ProcessPendingRequest();
}

/** Only available during game play */
APawn* FUnrealcvServer::GetPawn()
{
	UWorld* World = GetGameWorld();
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

/**
 For UnrealCV server, when a game start:
 1. Start a TCPserver.
 2. Create a command dispatcher
 3. Add command handler to command dispatcher, CameraHandler should be able to access camera
 4. Bind command dispatcher to TCPserver
 5. Bind command dispatcher to UE4 console

 When a new pawn is created.
 1. Update this pawn with GTCaptureComponent
 */

void FUnrealcvServer::RegisterCommandHandlers()
{
	// Taken from ctor, because might cause loop-invoke.
	CommandHandlers.Add(new FObjectCommandHandler());
	CommandHandlers.Add(new FPluginCommandHandler());
	CommandHandlers.Add(new FActionCommandHandler());
	CommandHandlers.Add(new FAliasCommandHandler());
	CommandHandlers.Add(new FSensorHandler());
	for (FCommandHandler* Handler : CommandHandlers)
	{
		Handler->CommandDispatcher = CommandDispatcher;
		Handler->RegisterCommands();
	}
}

FUnrealcvServer::FUnrealcvServer()
{
	// Code defined here should not use FUnrealcvServer::Get();
	NetworkManager = NewObject<UNetworkManager>();
	CommandDispatcher = TSharedPtr<FCommandDispatcher>(new FCommandDispatcher());
	FConsoleHelper::Get().SetCommandDispatcher(CommandDispatcher);

	NetworkManager->AddToRoot(); // Avoid GC
	NetworkManager->OnReceived().AddRaw(this, &FUnrealcvServer::HandleRawMessage);
	NetworkManager->OnError().AddRaw(this, &FUnrealcvServer::HandleError);
}

FUnrealcvServer::~FUnrealcvServer()
{
	// this->NetworkManager->FinishDestroy(); // TODO: Check is this usage correct?
}

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
	UWorld *World = GetGameWorld();
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

		FObjectPainter::Get().Reset(GetPawn()->GetLevel());
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
// 	APlayerController* PlayerController = GetGameWorld()->GetFirstPlayerController();
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
// 	UGameplayStatics::OpenLevel(GetGameWorld(), LevelName);
// 	UGameplayStatics::FlushLevelStreaming(GetGameWorld());
// 	UE_LOG(LogUnrealCV, Warning, TEXT("Level loaded"));
// }

// Each tick of GameThread.
void FUnrealcvServer::ProcessPendingRequest()
{
	while (!PendingRequest.IsEmpty())
	{
		// if (!InitWorld()) break;

		FRequest Request;
		bool DequeueStatus = PendingRequest.Dequeue(Request);
		check(DequeueStatus);
		int32 RequestId = Request.RequestId;

		FCallbackDelegate CallbackDelegate;
		CallbackDelegate.BindLambda([this, RequestId](FExecStatus ExecStatus)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Response: %s"), *ExecStatus.GetMessage());

			FString Header = FString::Printf(TEXT("%d:"), RequestId);
			TArray<uint8> ReplyData;
			FExecStatus::BinaryArrayFromString(Header, ReplyData);

			ReplyData += ExecStatus.GetData();
			NetworkManager->SendData(ReplyData);
		});
		CommandDispatcher->ExecAsync(Request.Message, CallbackDelegate);
	}
}

/** Message handler for server */
void FUnrealcvServer::HandleRawMessage(const FString& InRawMessage)
{
	UE_LOG(LogUnrealCV, Warning, TEXT("Request: %s"), *InRawMessage);
	// Parse Raw Message
	FString MessageFormat = "(\\d{1,8}):(.*)";
	FRegexPattern RegexPattern(MessageFormat);
	FRegexMatcher Matcher(RegexPattern, InRawMessage);

	if (Matcher.FindNext())
	{
		// TODO: Handle malform request message
		FString StrRequestId = Matcher.GetCaptureGroup(1);
		FString Message = Matcher.GetCaptureGroup(2);

		uint32 RequestId = FCString::Atoi(*StrRequestId);
		FRequest Request(Message, RequestId);
		this->PendingRequest.Enqueue(Request);
	}
	else
	{
		SendClientMessage(FString::Printf(TEXT("error: Malformat raw message '%s'"), *InRawMessage));
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

void FUnrealcvServer::SendClientMessage(FString Message)
{
	// TODO: Do not use game thread to send message.
	NetworkManager->SendMessage(Message);
}
