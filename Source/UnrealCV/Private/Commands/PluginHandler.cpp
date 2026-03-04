#include "PluginHandler.h"
#include "Engine/World.h"
#include "Interfaces/IPluginManager.h"

#include "UnrealcvServer.h"
#include "UnixTcpServer.h"
#include "UnrealcvShim.h"
#include "UnrealcvStats.h"

DECLARE_CYCLE_STAT(TEXT("FPluginHandler::GetUnrealCVStatus"),
	STAT_GetUnrealCVStatus, STATGROUP_UnrealCV);

FExecStatus FPluginHandler::Echo(const TArray<FString>& Args)
{
	if (Args.Num() < 1) { return FExecStatus::InvalidArgument; }
	// Async version
	const FString Msg = Args[0];
	// FPromiseDelegate PromiseDelegate = FPromiseDelegate::CreateLambda([Msg]()
	// {
	// 	return FExecStatus::OK(Msg);
	// });
	// return FExecStatus::AsyncQuery(FPromise(PromiseDelegate));

	// Sync version
	return FExecStatus::OK(Args[0]);
}

FExecStatus FPluginHandler::GetUnrealCVStatus(const TArray<FString>& Args)
{
	SCOPE_CYCLE_COUNTER(STAT_GetUnrealCVStatus);
	const FUnrealcvServer& Server = FUnrealcvServer::Get();
	const UUnixTcpServer* Tcp = Server.GetTcpServer();

	FString Msg;
	Msg += Tcp->IsListening()  ? TEXT("Is Listening\n")      : TEXT("Not Listening\n");
	Msg += Tcp->IsConnected()  ? TEXT("Client Connected\n")  : TEXT("No Client Connected\n");
	Msg += FString::Printf(TEXT("%d\n"), Tcp->GetPortNum());
	Msg += TEXT("Configuration\n");
	Msg += Server.GetConfig().ToString();
	return FExecStatus::OK(Msg);
}


FExecStatus FPluginHandler::GetCommands(const TArray<FString>& Args)
{
	FString Message;

	TArray<FString> UriList;
	TMap<FString, FString> UriDescription = CommandDispatcher->GetUriDescription();
	UriDescription.GetKeys(UriList);

	for (const auto& Value : UriDescription)
	{
		Message += Value.Key + "\n";
		Message += Value.Value + "\n";
	}

	return FExecStatus::OK(Message);
}

FExecStatus FPluginHandler::GetVersion(const TArray<FString>& Args)
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("UnrealCV");
	if (!Plugin.IsValid())
	{
		return FExecStatus::Error("The plugin is not correctly loaded");
	}
	const FPluginDescriptor& Descriptor = Plugin->GetDescriptor();
	return FExecStatus::OK(Descriptor.VersionName);
}

FExecStatus FPluginHandler::GetSceneName(const TArray<FString>& Args)
{
	FString SceneName = GetProjectName();
	return FExecStatus::OK(SceneName);
}

FExecStatus FPluginHandler::GetLevelName(const TArray<FString>& Args)
{
	const FUnrealcvServer& Server = FUnrealcvServer::Get();
	const UWorld* World = Server.GetWorld();
	if (!IsValid(World))
	{
		return FExecStatus::Error(TEXT("No valid world"));
	}
	const FString PrefixedMapName = World->GetMapName();
	const FString Prefix = World->StreamingLevelsPrefix;
	return FExecStatus::OK(PrefixedMapName.Mid(Prefix.Len()));
}

void FPluginHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginHandler::GetUnrealCVStatus);
	Help = "Get the status of UnrealCV plugin";
	CommandDispatcher->BindCommand("vget /unrealcv/status", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginHandler::GetCommands);
	Help = "List all available commands and their help message";
	CommandDispatcher->BindCommand(TEXT("vget /unrealcv/help"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginHandler::Echo);
	Help = "[debug] Echo back all message, for debug";
	CommandDispatcher->BindCommand(TEXT("vget /unrealcv/echo [str]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginHandler::GetVersion);
	Help = "Get the version of UnrealCV, the format is v0.*.*";
	CommandDispatcher->BindCommand(TEXT("vget /unrealcv/version"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginHandler::GetSceneName);
	Help = "Get the name of this scene, to make sure the annotation data is for this scene.";
	CommandDispatcher->BindCommand(TEXT("vget /scene/name"), Cmd, Help);

	CommandDispatcher->BindCommand(
		"vget /level/name",
		FDispatcherDelegate::CreateRaw(this, &FPluginHandler::GetLevelName),
		"Get current level name"
	);
}
