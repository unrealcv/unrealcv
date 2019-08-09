#include "PluginHandler.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"

#include "UnrealcvServer.h"
#include "UnrealcvShim.h"
#include "UnrealcvStats.h"

DECLARE_CYCLE_STAT(TEXT("FPluginHandler::GetUnrealCVStatus"), 
	STAT_GetUnrealCVStatus, STATGROUP_UnrealCV);

FExecStatus FPluginHandler::Echo(const TArray<FString>& Args)
{
	// Async version
	FString Msg = Args[0];
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
	FString Msg;
	FUnrealcvServer& Server = FUnrealcvServer::Get(); // TODO: Can not use a copy constructor, need to disable copy constructor

	if (Server.TcpServer->IsListening())
	{
		Msg += "Is Listening\n";
	}
	else
	{
		Msg += "Not Listening\n";
	}
	if (Server.TcpServer->IsConnected())
	{
		Msg += "Client Connected\n";
	}
	else
	{
		Msg += "No Client Connected\n";
	}
	Msg += FString::Printf(TEXT("%d\n"), Server.TcpServer->PortNum);
	Msg += "Configuration\n";
	Msg += FUnrealcvServer::Get().Config.ToString();
	return FExecStatus::OK(Msg);
}


FExecStatus FPluginHandler::GetCommands(const TArray<FString>& Args)
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

FExecStatus FPluginHandler::GetVersion(const TArray<FString>& Args)
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("UnrealCV");
	if (!Plugin.IsValid())
	{
		return FExecStatus::Error("The plugin is not correctly loaded");
	}
	else
	{
		FString PluginName = Plugin->GetName();
		FPluginDescriptor PluginDescriptor = Plugin->GetDescriptor();
		FString VersionName = PluginDescriptor.VersionName;
		int32 VersionNumber = PluginDescriptor.Version;
		return FExecStatus::OK(VersionName);
	}
}

FExecStatus FPluginHandler::GetSceneName(const TArray<FString>& Args)
{
	FString SceneName = GetProjectName();
	return FExecStatus::OK(SceneName);
}

FExecStatus FPluginHandler::GetLevelName(const TArray<FString>& Args)
{
	FUnrealcvServer& Server = FUnrealcvServer::Get();
	FString PrefixedMapName = Server.GetWorld()->GetMapName();	
	FString Prefix = Server.GetWorld()->StreamingLevelsPrefix;
	FString MapName = PrefixedMapName.Mid(Prefix.Len());
	return FExecStatus::OK(MapName);
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
