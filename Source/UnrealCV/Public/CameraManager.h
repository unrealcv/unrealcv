#pragma once
#include "GTCaptureComponent.h"

class FCameraManager
{
private:
	FCameraManager() {}
	TArray<UGTCaptureComponent*> CaptureComponentList;

public:
	static FCameraManager& Get()
	{
		static FCameraManager Singleton;
		return Singleton;
	};

	UGTCaptureComponent* GetCamera(int32 CameraId);
	void AttachGTCaptureComponentToCamera(APawn* Pawn);
	TArray<UGTCaptureComponent*> GetCameraList();
};
