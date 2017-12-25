#pragma once
#include "GTCaptureComponent.h"

/** [Deprecated] Manage all cameras of the scene */
class FCaptureManager
{
private:
	FCaptureManager() {}
	TArray<UGTCaptureComponent*> CaptureComponentList;

public:
	void AttachGTCaptureComponentToCamera(APawn* Pawn);
	static FCaptureManager& Get()
	{
		static FCaptureManager Singleton;
		return Singleton;
	};
	UGTCaptureComponent* GetCamera(int32 CameraId);
};
