#pragma once
#include "UE4CVServer.h"
#include "Engine.h"
#include "GTCaptureComponent.generated.h"

struct FGTCaptureTask
{
	FString Mode;
	FString Filename;
	uint64 CurrentFrame;
	FAsyncRecord* AsyncRecord;
	FGTCaptureTask() {}
	FGTCaptureTask(FString InMode, FString InFilename, uint64 InCurrentFrame, FAsyncRecord* InAsyncRecord) :
		Mode(InMode), Filename(InFilename), CurrentFrame(InCurrentFrame), AsyncRecord(InAsyncRecord) {}
};

/**
 * Use USceneCaptureComponent2D to export information from the scene.
 * This class needs to be tickable to update the rotation of the USceneCaptureComponent2D
 */
UCLASS()
class UNREALCV_API UGTCaptureComponent : public USceneComponent // , public FTickableGameObject
{
	GENERATED_BODY()
private:
	UGTCaptureComponent();
	APawn* Pawn;

	TArray<uint8> NpySerialization(TArray<FColor> ImageData, int32 Width, int32 Height, int32 Channel);
	TArray<uint8> NpySerialization(TArray<FFloat16Color> ImageData, int32 Width, int32 Height, int32 Channel);

public:
	static UGTCaptureComponent* Create(APawn* Pawn, TArray<FString> Modes);

	static UMaterial* GetMaterial(FString ModeName);

	// virtual void Tick(float DeltaTime) override; // TODO
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override; // TODO

	void SetFOVAngle(float FOV);

	/** Save image to a file */
	FAsyncRecord* Capture(FString Mode, FString Filename);

	/** Read binary data in png format */
	TArray<uint8> CapturePng(FString Mode);

	/** Read binary data in uncompressed numpy array */
	TArray<uint8> CaptureNpyUint8(FString Mode, int32 Channels);

	/** Read binary data in uncompressed numpy array */
	TArray<uint8> CaptureNpyFloat16(FString Mode, int32 Channels);

	USceneCaptureComponent2D* GetCaptureComponent(FString Mode);

private:
	const bool bIsTicking = true;

	TQueue<FGTCaptureTask, EQueueMode::Spsc> PendingTasks;
	TMap<FString, USceneCaptureComponent2D*> CaptureComponents;
};
