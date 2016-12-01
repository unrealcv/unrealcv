#include "UnrealCVPrivate.h"

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
	Server.NetworkManager->Start(Server.Config.Port);
}

void FUnrealCVPlugin::ShutdownModule()
{
}
