#pragma once
#include "GTCaptureComponent.h"

class FCaptureManager
{
private:
	FCaptureManager() {}
	TArray<UGTCaptureComponent*> CaptureComponentList;
	void AttachGTCaptureComponentToCamera(APawn* Pawn);

public:
	static FCaptureManager& Get()
	{
		static FCaptureManager Singleton;
		return Singleton;
	};
	UGTCaptureComponent* GetCamera(int32 CameraId);
};
