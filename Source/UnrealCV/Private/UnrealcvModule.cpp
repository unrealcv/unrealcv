#include "Runtime/Core/Public/Misc/CommandLine.h"
#include "Runtime/Core/Public/Misc/Parse.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/Core/Public/Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

#include "UnrealcvServer.h"
#include "UnrealcvLog.h"

DEFINE_LOG_CATEGORY(LogUnrealCV);

class FUnrealCVPlugin : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FUnrealCVPlugin, UnrealCV)

void FUnrealCVPlugin::StartupModule()
{
	FString Commandline = FCommandLine::Get();
	if (IsRunningDedicatedServer() ||
		Commandline.Contains(TEXT("cookcommandlet")) ||
		Commandline.Contains(TEXT("run=cook")))
	{
		// Do no start unrealcv module if running in the commandlet mode.
		return;
	}

	FUnrealcvServer &Server = FUnrealcvServer::Get();
	Server.RegisterCommandHandlers();

	int OverridePort = Server.Config.Port;
	if (FParse::Value(FCommandLine::Get(), TEXT("UnrealCVPort"), OverridePort)) {
	UE_LOG(LogUnrealCV, Warning, TEXT("Overriding listening port to %d"), OverridePort);
		Server.Config.Port = OverridePort;
	}

	int OverrideWidth = Server.Config.Width;
	if (FParse::Value(FCommandLine::Get(), TEXT("UnrealCVWidth"), OverrideWidth)) {
	UE_LOG(LogUnrealCV, Warning, TEXT("Overriding width to %d"), OverrideWidth);
		Server.Config.Width = OverrideWidth;
	}

	int OverrideHeight = Server.Config.Height;
	if (FParse::Value(FCommandLine::Get(), TEXT("UnrealCVHeight"), OverrideHeight)) {
	UE_LOG(LogUnrealCV, Warning, TEXT("Overriding height to %d"), OverrideHeight);
		Server.Config.Height = OverrideHeight;
	}

	float OverrideFOV = Server.Config.FOV;
	if (FParse::Value(FCommandLine::Get(), TEXT("UnrealCVFOV"), OverrideFOV)) {
	UE_LOG(LogUnrealCV, Warning, TEXT("Overriding FOV to %f"), OverrideFOV);
		Server.Config.FOV = OverrideFOV;
	}

	bool OverrideEnableInput = Server.Config.EnableInput;
	if (FParse::Bool(FCommandLine::Get(), TEXT("UnrealCVEnableInput"), OverrideEnableInput)) {
		if (OverrideEnableInput)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Overriding EnableInput to true"));
		}
		else
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Overriding EnableInput to false"));
		}
		Server.Config.EnableInput = OverrideEnableInput;
	}

	bool OverrideExitOnFailure = Server.Config.ExitOnFailure;
	if (FParse::Bool(FCommandLine::Get(), TEXT("UnrealCVExitOnFailure"), OverrideExitOnFailure)) {
		if (OverrideExitOnFailure)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Overriding ExitOnFailure to true"));
		}
		else
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Overriding ExitOnFailure to false"));
		}
		Server.Config.ExitOnFailure = OverrideExitOnFailure;
	}

	bool StartSuccess = Server.TcpServer->Start(Server.Config.Port);
	if (!StartSuccess)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Failed to start network server"));
		if (Server.Config.ExitOnFailure)
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Requesting exit"));
			FGenericPlatformMisc::RequestExit(false);
		}
	}

	// Inject a UObject into the world to listen for world event
}

void FUnrealCVPlugin::ShutdownModule()
{
}

