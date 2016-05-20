#pragma once

#include "ModuleManager.h"

class IUnrealCVPlugin : public IModuleInterface
{
public:
	static inline IUnrealCVPlugin& Get()
	{
		return FModuleManager::LoadModuleChecked<IUnrealCVPlugin>("UnrealCV");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("UnrealCV");
	}
};