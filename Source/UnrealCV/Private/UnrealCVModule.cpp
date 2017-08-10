#include "UnrealCVPrivate.h"
#include "Runtime/Core/Public/Misc/CommandLine.h"
#include "Runtime/Core/Public/Misc/Parse.h"

DEFINE_LOG_CATEGORY(LogUnrealCV);

class FUnrealCVPlugin : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FUnrealCVPlugin, UnrealCV)

void FUnrealCVPlugin::StartupModule()
{
	FUE4CVServer &Server = FUE4CVServer::Get();
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

	Server.NetworkManager->Start(Server.Config.Port);
}

void FUnrealCVPlugin::ShutdownModule()
{
}
