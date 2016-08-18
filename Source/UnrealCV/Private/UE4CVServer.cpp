// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCVPrivate.h"
#include "UE4CVServer.h"
#include "CameraManager.h"
#include "PlayerViewMode.h"

void FUE4CVServer::BeginPlay()
{
	// This should be done after the world is initialized.
	FConsoleHelper::Get().RegisterConsole();
	
	APlayerController* PlayerController = GWorld->GetFirstPlayerController();
	check(PlayerController);
	APawn* Pawn = PlayerController->GetPawn();
	check(Pawn);
	this->Pawn = Pawn;

	FObjectPainter::Get().SetLevel(Pawn->GetLevel());
	// TODO: Check the pointers
	FObjectPainter::Get().PaintRandomColors();

	FCameraManager::Get().AttachGTCaptureComponentToCamera(Pawn);

	FPlayerViewMode::Get().CreatePostProcessVolume();
	FPlayerViewMode::Get().Lit();

	bIsTicking = true;
	NetworkManager->Start(); // Do not process any request if the game is in the stop mode.
}


/** Only available during game play */
APawn* FUE4CVServer::GetPawn()
{
	check(this->Pawn);
	return this->Pawn;
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

// Each tick of GameThread.
void FUE4CVServer::ProcessPendingRequest()
{
	while (!PendingRequest.IsEmpty())
	{
		FRequest Request;
		bool DequeueStatus = PendingRequest.Dequeue(Request);
		check(DequeueStatus);
		int32 RequestId = Request.RequestId;

		FCallbackDelegate CallbackDelegate;
		CallbackDelegate.BindLambda([this, RequestId](FExecStatus ExecStatus)
		{
			UE_LOG(LogTemp, Warning, TEXT("Response: %s"), *ExecStatus.GetMessage());
			FString ReplyRawMessage = FString::Printf(TEXT("%d:%s"), RequestId, *ExecStatus.GetMessage());
			SendClientMessage(ReplyRawMessage);
		});
		CommandDispatcher->ExecAsync(Request.Message, CallbackDelegate);
	}
}

/** Message handler for server */
void FUE4CVServer::HandleRawMessage(const FString& InRawMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Request: %s"), *InRawMessage);
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
