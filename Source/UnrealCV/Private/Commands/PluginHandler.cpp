#include "UnrealCVPrivate.h"
#include "PluginHandler.h"
#include "UE4CVServer.h"

void FPluginCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;
	
	// Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::GetPort);
	// CommandDispatcher->BindCommand("vget /unrealcv/port", Cmd, "Get port from the plugin listening to");
	// 
	// Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::SetPort);
	// CommandDispatcher->BindCommand("vset /unrealcv/port [uint]", Cmd, "Set port the plugin listening to");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::GetUnrealCVStatus);
	Help = "Get the status of UnrealCV plugin";
	CommandDispatcher->BindCommand("vget /unrealcv/status", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::GetCommands);
	Help = "List all available commands and their help message";
	CommandDispatcher->BindCommand(TEXT("vget /unrealcv/help"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::Echo);
	Help = "[debug] Echo back all message, for debug";
	CommandDispatcher->BindCommand(TEXT("vget /unrealcv/echo [str]"), Cmd, Help);
}

FExecStatus FPluginCommandHandler::Echo(const TArray<FString>& Args)
{
	// Async version
	FString Msg = Args[0];
	FPromiseDelegate PromiseDelegate = FPromiseDelegate::CreateLambda([Msg]()
	{
		return FExecStatus::OK(Msg);
	});
	return FExecStatus::AsyncQuery(FPromise(PromiseDelegate));
	// Sync version
	// return FExecStatus::OK(Args[0]);
}

FExecStatus FPluginCommandHandler::GetPort(const TArray<FString>& Args)
{
	FString Msg = FString::Printf(TEXT("%d"), FUE4CVServer::Get().NetworkManager->PortNum);
	return FExecStatus::OK(Msg);
	// return FExecStatus::InvalidArgument;
}

FExecStatus FPluginCommandHandler::SetPort(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 PortNum = FCString::Atoi(*Args[0]);
		bool IsSuccessful = FUE4CVServer::Get().NetworkManager->Start(PortNum);
		if (IsSuccessful)
		{
			return FExecStatus::OK();
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Fail to set port to %d"), PortNum));
		}
	}
	else
	{
		return FExecStatus::InvalidArgument;
	}
}

FExecStatus FPluginCommandHandler::GetUnrealCVStatus(const TArray<FString>& Args)
{
	FString Msg;
	FUE4CVServer& Server = FUE4CVServer::Get(); // TODO: Can not use a copy constructor, need to disable copy constructor
	if (Server.NetworkManager->IsListening())
	{
		Msg += "Is Listening\n";
	}
	else
	{
		Msg += "Not Listening\n";
	}
	if (Server.NetworkManager->IsConnected())
	{
		Msg += "Client Connected\n";
	}
	else
	{
		Msg += "No Client Connected\n";
	}
	Msg += FString::Printf(TEXT("%d\n"), Server.NetworkManager->PortNum);
	return FExecStatus::OK(Msg);
}


FExecStatus FPluginCommandHandler::GetCommands(const TArray<FString>& Args)
{
	FString Message;

	TArray<FString> UriList;
	TMap<FString, FString> UriDescription = CommandDispatcher->GetUriDescription();
	UriDescription.GetKeys(UriList);

	for (auto Value : UriDescription)
	{
		Message += Value.Key + "\n";
		Message += Value.Value + "\n";
	}

	return FExecStatus::OK(Message);
}
