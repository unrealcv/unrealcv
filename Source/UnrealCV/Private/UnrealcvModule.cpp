#include "Runtime/Core/Public/Misc/CommandLine.h"
#include "Runtime/Core/Public/Misc/Parse.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Core/Public/Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#include "UnrealcvServer.h"
#include "UnrealcvLog.h"

class FUnrealCVPlugin : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FUnrealCVPlugin, UnrealCV)

void FUnrealCVPlugin::StartupModule()
{
	const FString Commandline = FCommandLine::Get();
	if (IsRunningDedicatedServer() ||
		Commandline.Contains(TEXT("cookcommandlet")) ||
		Commandline.Contains(TEXT("run=cook")))
	{
		// Do not start unrealcv module if running in commandlet mode.
		return;
	}

	FUnrealcvServer& Server = FUnrealcvServer::Get();
	Server.RegisterCommandHandlers();

	FServerConfig& Config = Server.GetMutableConfig();

	int32 OverridePort = Config.Port;
	if (FParse::Value(FCommandLine::Get(), TEXT("UnrealCVPort"), OverridePort))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Overriding listening port to %d"), OverridePort);
		Config.Port = OverridePort;
	}

	int32 OverrideWidth = Config.Width;
	if (FParse::Value(FCommandLine::Get(), TEXT("UnrealCVWidth"), OverrideWidth))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Overriding width to %d"), OverrideWidth);
		Config.Width = OverrideWidth;
	}

	int32 OverrideHeight = Config.Height;
	if (FParse::Value(FCommandLine::Get(), TEXT("UnrealCVHeight"), OverrideHeight))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Overriding height to %d"), OverrideHeight);
		Config.Height = OverrideHeight;
	}

	float OverrideFOV = Config.FOV;
	if (FParse::Value(FCommandLine::Get(), TEXT("UnrealCVFOV"), OverrideFOV))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Overriding FOV to %f"), OverrideFOV);
		Config.FOV = OverrideFOV;
	}

	bool bOverrideEnableInput = Config.bEnableInput;
	if (FParse::Bool(FCommandLine::Get(), TEXT("UnrealCVEnableInput"), bOverrideEnableInput))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Overriding EnableInput to %s"), bOverrideEnableInput ? TEXT("true") : TEXT("false"));
		Config.bEnableInput = bOverrideEnableInput;
	}

	bool bOverrideExitOnFailure = Config.bExitOnFailure;
	if (FParse::Bool(FCommandLine::Get(), TEXT("UnrealCVExitOnFailure"), bOverrideExitOnFailure))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Overriding ExitOnFailure to %s"), bOverrideExitOnFailure ? TEXT("true") : TEXT("false"));
		Config.bExitOnFailure = bOverrideExitOnFailure;
	}

	const bool bStartSuccess = Server.GetTcpServer()->Start(Config.Port);
	if (!bStartSuccess)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Failed to start network server"));
		if (Config.bExitOnFailure)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Requesting exit"));
			FGenericPlatformMisc::RequestExit(false);
		}
	}
}

void FUnrealCVPlugin::ShutdownModule()
{
}

