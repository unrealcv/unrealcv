#include "UnrealCVPrivate.h"
#include "CaptureManager.h"

/**
  * Where to put cameras
  * For each camera in the scene, attach SceneCaptureComponent2D to it
  * So that ground truth can be generated, invoked when a new actor is created
  */
void FCaptureManager::AttachGTCaptureComponentToCamera(APawn* Pawn)
{
	// TODO: Only support one camera at the beginning
	// TODO: Make this automatic from material loader.
	TArray<FString> SupportedModes;
	SupportedModes.Add(TEXT("lit")); // This is lit
	SupportedModes.Add(TEXT("depth"));
	SupportedModes.Add(TEXT("debug"));
	SupportedModes.Add(TEXT("object_mask"));
	SupportedModes.Add(TEXT("normal")); 
	SupportedModes.Add(TEXT("wireframe"));
	SupportedModes.Add(TEXT("default")); 
	// TODO: Get the list from GTCaptureComponent

	CaptureComponentList.Empty();

	UGTCaptureComponent* Capturer = UGTCaptureComponent::Create(Pawn, SupportedModes);
	CaptureComponentList.Add(Capturer);

	UGTCaptureComponent* RightEye = UGTCaptureComponent::Create(Pawn, SupportedModes);
	RightEye->AddLocalOffset(FVector(0, 40, 0)); // TODO: make this configurable
	CaptureComponentList.Add(RightEye);
}

APawn* GetFirstPawn()
{
	static UWorld* CurrentWorld = nullptr;
	static APawn* Pawn = nullptr;
	if (Pawn == nullptr || CurrentWorld != GWorld)
	{
		APlayerController* PlayerController = GWorld->GetFirstPlayerController();
		check(PlayerController);
		Pawn = PlayerController->GetPawn();
		check(Pawn);
		CurrentWorld = GWorld;
	}
	return Pawn;
}

UGTCaptureComponent* FCaptureManager::GetCamera(int32 CameraId)
{
	static UWorld* CurrentWorld = nullptr;
	if (CaptureComponentList.Num() == 0 || CurrentWorld != GWorld)
	{
		AttachGTCaptureComponentToCamera(GetFirstPawn());
		CurrentWorld = GWorld;
	}

	if (CameraId < CaptureComponentList.Num() && CameraId >= 0)
	{
		return CaptureComponentList[CameraId];
	}
	else
	{
		return nullptr;
	}
}