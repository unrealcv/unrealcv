#include "UnrealCVPrivate.h"
#include "CameraManager.h"

/**
  * Where to put cameras
  * For each camera in the scene, attach SceneCaptureComponent2D to it
  * So that ground truth can be generated, invoked when a new actor is created
  */
void FCameraManager::AttachGTCaptureComponentToCamera(APawn* Pawn)
{
	// TODO: Only support one camera at the beginning
	// TODO: Make this automatic from material loader.
	TArray<FString> SupportedModes;
	SupportedModes.Add(TEXT("debug"));
	SupportedModes.Add(TEXT("depth"));
	SupportedModes.Add(TEXT("object_mask"));
	SupportedModes.Add(TEXT("lit")); // This is lit

	UGTCaptureComponent* Capturer = UGTCaptureComponent::Create(Pawn, SupportedModes);
	CaptureComponentList.Add(Capturer);

	UGTCaptureComponent* RightEye = UGTCaptureComponent::Create(Pawn, SupportedModes);
	RightEye->AddLocalOffset(FVector(0, 40, 0)); // TODO: make this configurable
	CaptureComponentList.Add(RightEye);
}


UGTCaptureComponent* FCameraManager::GetCamera(int32 CameraId)
{
	check(CaptureComponentList.Num() != 0);
	if (CameraId < CaptureComponentList.Num() && CameraId >= 0)
	{
		return CaptureComponentList[CameraId];
	}
	else
	{
		return nullptr;
	}
}