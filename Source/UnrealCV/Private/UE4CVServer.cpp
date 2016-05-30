// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCVPrivate.h"
#include "UE4CVServer.h"
#include "UnrealCV.h"

bool FUE4CVServer::Init(APawn* InCharacter)
{
	FObjectPainter::Get().SetLevel(InCharacter->GetLevel());
	FObjectPainter::Get().PaintRandomColors();

	FViewMode::Get().SetWorld(InCharacter->GetWorld());

	CommandDispatcher = new FCommandDispatcher();
	UE4CVCommands* Commands = new UE4CVCommands(InCharacter, CommandDispatcher);
	// Register a set of commands to the command dispatcher

	FConsoleOutputDevice* ConsoleOutputDevice = new FConsoleOutputDevice(InCharacter->GetWorld()->GetGameViewport()->ViewportConsole); // TODO: Check the pointers
	FConsoleHelper* ConsoleHelper = new FConsoleHelper(CommandDispatcher, ConsoleOutputDevice);

	return NetworkManager->Start();
}

FUE4CVServer& FUE4CVServer::Get()
{
	static FUE4CVServer Singleton;
	return Singleton;
}

FUE4CVServer::FUE4CVServer()
{
	NetworkManager = NewObject<UNetworkManager>();
	NetworkManager->AddToRoot(); // Avoid GC
	NetworkManager->OnReceived().AddRaw(this, &FUE4CVServer::HandleRawMessage);
}

FUE4CVServer::~FUE4CVServer()
{
	this->NetworkManager->FinishDestroy(); // TODO: Check is this usage correct?
}

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

bool FUE4CVServer::Start()
{
	NetworkManager->Start();
	return true;
}

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

