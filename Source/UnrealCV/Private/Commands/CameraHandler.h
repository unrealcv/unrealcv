#pragma once
#include "CommandHandler.h"
#include "GTCaptureComponent.h"

class FScreenCapture
{
public:
	static FExecStatus GetCameraViewAsyncQuery(const FString& FullFilename);
};

class FCameraCommandHandler : public FCommandHandler
{
public:
	FCameraCommandHandler(FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCommandDispatcher)
	{}
	void RegisterCommands();

	/** vget /camera/location */
	FExecStatus GetCameraLocation(const TArray<FString>& Args);
	/** vset /camera/location */
	FExecStatus	SetCameraLocation(const TArray<FString>& Args);
	/** vset /camera/moveto */
	FExecStatus	MoveTo(const TArray<FString>& Args);
	/** vget /camera/rotation */
	FExecStatus GetCameraRotation(const TArray<FString>& Args);
	/** vset /camera/rotation */
	FExecStatus	SetCameraRotation(const TArray<FString>& Args);
	/** vget /camera/view */
	FExecStatus GetScreenshot(const TArray<FString>& Args);


	/** Get camera image with a given mode */
	FExecStatus GetCameraViewMode(const TArray<FString>& Args);

	/** Get camera project matrix */
	FExecStatus GetCameraProjMatrix(const TArray<FString>& Args);

	/** Get HDR buffer visualization */
	FExecStatus GetBuffer(const TArray<FString>& Args);

	/** Get HDR from the scene */
	// FExecStatus GetCameraHDR(const TArray<FString>& Args);

	// FExecStatus GetCameraDepth(const TArray<FString>& Args);
	// FExecStatus GetCameraLit(const TArray<FString>& Args);
};
