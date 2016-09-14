#pragma once
#include "GTCaptureComponent.h"

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
