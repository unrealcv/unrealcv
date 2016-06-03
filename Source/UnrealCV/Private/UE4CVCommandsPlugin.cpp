#include "UnrealCVPrivate.h"
#include "UE4CVCommands.h"
#include "UE4CVServer.h"

void UE4CVCommands::RegisterCommandsPlugin()
{
	FDispatcherDelegate Cmd;
	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetPort);
	CommandDispatcher->BindCommand("vget /unrealcv/port", Cmd, "Get port from the plugin listening to");
	
	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::SetPort);
	CommandDispatcher->BindCommand("vset /unrealcv/port [uint]", Cmd, "Set port the plugin listening to");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetUnrealCVStatus);
	CommandDispatcher->BindCommand("vget /unrealcv/status", Cmd, "Get camera location");
}

FExecStatus UE4CVCommands::GetPort(const TArray<FString>& Args)
{
	FString Msg = FString::Printf(TEXT("%d"), FUE4CVServer::Get().NetworkManager->PortNum);
	return FExecStatus::OK(Msg);
	// return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::SetPort(const TArray<FString>& Args)
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

FExecStatus UE4CVCommands::GetUnrealCVStatus(const TArray<FString>& Args)
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
