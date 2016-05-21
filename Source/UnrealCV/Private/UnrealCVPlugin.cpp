#include "UnrealCVPrivate.h"

class FUnrealCVPlugin : public IUnrealCVPlugin
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FUnrealCVPlugin, UnrealCV)

void FUnrealCVPlugin::StartupModule()
{

}

void FUnrealCVPlugin::ShutdownModule()
{

}