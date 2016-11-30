#include "UnrealCVPrivate.h"
#include "UE4CVServer.h"
#include "PlayerViewMode.h"
#include "ConsoleHelper.h"
#include "ObjectPainter.h"
#include "CaptureManager.h"
#include "CameraHandler.h"
#include "ObjectHandler.h"
#include "PluginHandler.h"
#include "UnrealEd.h"

/** Only available during game play */
APawn* FUE4CVServer::GetPawn()
{
	static UWorld* CurrentWorld = nullptr;
	UWorld* World = GetGameWorld();
	if (CurrentWorld != World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		check(PlayerController);
		Pawn = PlayerController->GetPawn();
		check(Pawn);
		CurrentWorld = World;
	}
	return Pawn;
}

FUE4CVServer& FUE4CVServer::Get()
{
	static FUE4CVServer Singleton;
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

void FUE4CVServer::RegisterCommandHandlers()
{
	// Taken from ctor, because might cause loop-invoke.
	CommandHandlers.Add(new FObjectCommandHandler(CommandDispatcher));
	CommandHandlers.Add(new FCameraCommandHandler(CommandDispatcher));
	CommandHandlers.Add(new FPluginCommandHandler(CommandDispatcher));
	for (FCommandHandler* Handler : CommandHandlers)
	{
		Handler->RegisterCommands();
	}
}

FUE4CVServer::FUE4CVServer()
{
	// Code defined here should not use FUE4CVServer::Get();
	NetworkManager = NewObject<UNetworkManager>();
	CommandDispatcher = new FCommandDispatcher();
	FConsoleHelper::Get().SetCommandDispatcher(CommandDispatcher);

	NetworkManager->AddToRoot(); // Avoid GC
	NetworkManager->OnReceived().AddRaw(this, &FUE4CVServer::HandleRawMessage);
}

FUE4CVServer::~FUE4CVServer()
{
	// this->NetworkManager->FinishDestroy(); // TODO: Check is this usage correct?
}

UWorld* FUE4CVServer::GetGameWorld()
{
	// The correct way to get GameWorld;
	UEditorEngine* EditorEngine = Cast<UEditorEngine>(GEngine); // TODO: check which macro can determine whether I am in editor
	UWorld* World = nullptr;
	if (EditorEngine != nullptr)
	{
		World = EditorEngine->PlayWorld;
		check(World->IsGameWorld());
		return World;
	}

	UGameEngine* GameEngine = Cast<UGameEngine>(GEngine);
	if (GameEngine != nullptr)
	{
		World = GameEngine->GetGameWorld();
		check(World->IsGameWorld());
		return World;
	}

	check(false);
	return World;
}


/**
 * Make sure the UE4CVServer is correctly configured.
 */
void FUE4CVServer::InitGWorld()
{
	UWorld *World = GetGameWorld();
	// Use this to replace BeginPlay()
	static UWorld *CurrentWorld = nullptr;
	if (CurrentWorld != World)
	{
		// Invoke this everytime when the GWorld changes
		// This will happen when the game is stopped and restart in the UE4Editor
		// APlayerController* PlayerController = World->GetFirstPlayerController();
		APlayerController* PlayerController = World->GetFirstPlayerController();
		check(PlayerController);
		FObjectPainter::Get().SetLevel(GetPawn()->GetLevel());
		FObjectPainter::Get().PaintRandomColors();

		FCaptureManager::Get().AttachGTCaptureComponentToCamera(GetPawn());

		CurrentWorld = World;
	}
}

// Each tick of GameThread.
void FUE4CVServer::ProcessPendingRequest()
{
	while (!PendingRequest.IsEmpty())
	{
		this->InitGWorld();

		FRequest Request;
		bool DequeueStatus = PendingRequest.Dequeue(Request);
		check(DequeueStatus);
		int32 RequestId = Request.RequestId;

		FCallbackDelegate CallbackDelegate;
		CallbackDelegate.BindLambda([this, RequestId](FExecStatus ExecStatus)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Response: %s"), *ExecStatus.GetMessage());
			FString ReplyRawMessage = FString::Printf(TEXT("%d:%s"), RequestId, *ExecStatus.GetMessage());
			SendClientMessage(ReplyRawMessage);
		});
		CommandDispatcher->ExecAsync(Request.Message, CallbackDelegate);
	}
}

/** Message handler for server */
void FUE4CVServer::HandleRawMessage(const FString& InRawMessage)
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

void FUE4CVServer::SendClientMessage(FString Message)
{
	// TODO: Do not use game thread to send message.
	NetworkManager->SendMessage(Message);
}
