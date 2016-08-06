#pragma once
#include "CommandHandler.h"
#include "GTCapturer.h"

class FCameraCommandHandler : public FCommandHandler
{
public:
	FCameraCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher)
	: FCommandHandler(InCharacter, InCommandDispatcher)
	{}
	void RegisterCommands();

	void InitCameraArray();

	TMap<FString, UGTCapturer*> GTCapturers; // TODO: Support multi-camera

	/** vget /camera/location */
	FExecStatus GetCameraLocation(const TArray<FString>& Args);
	/** vset /camera/location */
	FExecStatus	SetCameraLocation(const TArray<FString>& Args);
	/** vget /camera/rotation */
	FExecStatus GetCameraRotation(const TArray<FString>& Args);
	/** vset /camera/rotation */
	FExecStatus	SetCameraRotation(const TArray<FString>& Args);
	/** vget /camera/view */
	FExecStatus GetCameraView(const TArray<FString>& Args);

	/** Deprecated: Get camera view in a sync way, can not work in standalone mode */
	FExecStatus GetCameraViewSync(const FString& Fullfilename);
	/** Get camera view in async way, the return FExecStaus is in pending status and need to check the promise to get result */
	FExecStatus GetCameraViewAsyncQuery(const FString& Fullfilename);
	// FExecStatus GetCameraViewAsyncCallback(const FString& Fullfilename);

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
