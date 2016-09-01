#include "UnrealCVPrivate.h"

class FUnrealCVPlugin : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FUnrealCVPlugin, UnrealCV)

void FUnrealCVPlugin::StartupModule()
{
	FUE4CVServer::Get().RegisterCommandHandlers();
	FUE4CVServer::Get().NetworkManager->Start();
}

void FUnrealCVPlugin::ShutdownModule()
{
	// TODO: implement stop
}
