#include "UnrealCVPrivate.h"

class FUnrealCVPlugin : public IModuleInterface
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FUnrealCVPlugin, UnrealCV)

void FUnrealCVPlugin::StartupModule()
{
	FUE4CVServer::Get().Start();
}

void FUnrealCVPlugin::ShutdownModule()
{
	// TODO: implement stop
}
