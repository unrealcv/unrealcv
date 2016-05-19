// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "UE4CVServer.h"

FUE4CVServer::FUE4CVServer(FCommandDispatcher* CommandDispatcher)
{
	this->CommandDispatcher = CommandDispatcher;
	// Command Dispatcher defines the behavior of the server, TODO: Write with javadoc style

	this->NetworkManager = NewObject<UNetworkManager>();
	this->NetworkManager->AddToRoot(); // Avoid GC

	this->NetworkManager->OnReceived().AddRaw(this, &FUE4CVServer::HandleRequest);
}

FUE4CVServer::~FUE4CVServer()
{
	this->NetworkManager->FinishDestroy(); // TODO: Check is this usage correct?
}

void FUE4CVServer::ProcessPendingRequest()
{
	if (PendingTask.Active) // Do query or wait for callback
	{
		FExecStatus ExecStatus = PendingTask.CheckStatus();
		if (ExecStatus == FExecStatusType::Pending)
		{
			return; // The task is still pending
		}
		else
		{
			PendingTask.Active = false;
			FString ReplyRawMessage = FString::Printf(TEXT("%d:%s"), PendingTask.RequestId, *ExecStatus.GetMessage());
			SendClientMessage(ReplyRawMessage);
		}
	}

	while (!PendingRequest.IsEmpty())
	{
		FRequest Request;
		bool DequeueStatus = PendingRequest.Dequeue(Request);
		check(DequeueStatus);

		FExecStatus ExecStatus = CommandDispatcher->Exec(Request.Message);
		if (ExecStatus == FExecStatusType::Pending)
		{
			PendingTask = FPendingTask(ExecStatus.Promise, Request.RequestId);
			break;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Response: %s"), *ExecStatus.GetMessage());
			FString ReplyRawMessage = FString::Printf(TEXT("%d:%s"), Request.RequestId, *ExecStatus.GetMessage());
			SendClientMessage(ReplyRawMessage);
		}
	}
}

bool FUE4CVServer::Start()
{
	NetworkManager->Start();
	return true;
}

void FUE4CVServer::HandleRequest(const FString& InRawMessage)
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

		/*
		FExecStatus ExecStatus = CommandDispatcher->Exec(Message);
		UE_LOG(LogTemp, Warning, TEXT("Response: %s"), *ExecStatus.Message);
		FString ReplyRawMessage = FString::Printf(TEXT("%d:%s"), RequestId, *ExecStatus.Message);
		SendClientMessage(ReplyRawMessage);
		*/
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

