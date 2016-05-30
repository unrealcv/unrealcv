#include "UnrealCVPrivate.h"
#include "UE4CVCommands.h"

void UE4CVCommands::RegisterCommandsPlugin()
{
	FDispatcherDelegate Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetPort);
	CommandDispatcher->BindCommand("vget /unrealcv/port", Cmd, "Get port from the plugin listening to");
	
	FDispatcherDelegate Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::SetPort);
	CommandDispatcher->BindCommand("vset /unrealcv/port", Cmd, "Set port the plugin listening to");

	FDispatcherDelegate Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetUnrealCVStatus);
	CommandDispatcher->BindCommand("vget /unrealcv/status", Cmd, "Get camera location");
}

FExecStatus UE4CVCommands::GetPort(const TArray<FString>& Args)
{

}

FExecStatus UE4CVCommands::SetPort(const TArray<FString>& Args)
{

}

FExecStatus UE4CVCommands::GetUnrealCVStatus(const TArray<FString>& Args)
{

}
