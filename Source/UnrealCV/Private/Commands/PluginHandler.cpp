#include "UnrealCVPrivate.h"
#include "PluginHandler.h"
#include "IPluginManager.h"
#include "UE4CVServer.h"

void FPluginCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::GetUnrealCVStatus);
	Help = "Get the status of UnrealCV plugin";
	CommandDispatcher->BindCommand("vget /unrealcv/status", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::GetCommands);
	Help = "List all available commands and their help message";
	CommandDispatcher->BindCommand(TEXT("vget /unrealcv/help"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::Echo);
	Help = "[debug] Echo back all message, for debug";
	CommandDispatcher->BindCommand(TEXT("vget /unrealcv/echo [str]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::GetVersion);
	Help = "Get the version of UnrealCV, the format is v0.*.*";
	CommandDispatcher->BindCommand(TEXT("vget /unrealcv/version"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FPluginCommandHandler::GetSceneName);
	Help = "Get the name of this scene, to make sure the annotation data is for this scene.";
	CommandDispatcher->BindCommand(TEXT("vget /scene/name"), Cmd, Help);

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
	Msg += "Configuration\n";
	Msg += FUE4CVServer::Get().Config.ToString();
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

FExecStatus FPluginCommandHandler::GetVersion(const TArray<FString>& Args)
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


FExecStatus FPluginCommandHandler::GetSceneName(const TArray<FString>& Args)
{
	FString SceneName = GetProjectName();
	return FExecStatus::OK(SceneName);
}
