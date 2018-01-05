// Weichao Qiu @ 2017
#pragma once

#include "CoreMinimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "ImageUtil.h"
#include "ImageWorker.h"
#include "BaseCameraSensor.generated.h"

/**
 *
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class UNREALCV_API UBaseCameraSensor : public USceneCaptureComponent2D
{
	GENERATED_BODY()
	// GENERATED_UCLASS_BODY()

public:
	UBaseCameraSensor(const FObjectInitializer& ObjectInitializer);
	// The order of this function declaration matters

	virtual void OnRegister() override;

	// 	FlushRenderingCommands() can make sure the rendering command is finished, but will slow down the game thread
	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	virtual void Capture(TArray<FColor>& ImageData, int& Width, int& Height);

	/** Save lit to an image file, send the capture command to rendering thread */
	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	virtual void CaptureAsync(const FString& Filename);

	/** The old version to read TextureBuffer, slow but is sync operation and  correct */
	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	virtual void CaptureSlow(TArray<FColor>& ImageData, int& Width, int& Height);

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	FVector GetSensorWorldLocation();

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	FRotator GetSensorRotation();

	// FString GetSensorPose();

	/** Get the FOV of this camera */
	FString GetSensorFOV();

	/** Get the projection matrix of this camera */
	FString GetProjectionMatrix();

	// void SetRotation();

	// void SetWorldLocation();

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	void SetFOV(float FOV);

	void SetPostProcessMaterial(UMaterial* PostProcessMaterial);


	/** The TextureBuffer width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnrealCV")
	int Width;

	/** The TextureBuffer height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UnrealCV")
	int Height;

	// UFUNCTION(BlueprintGetter, Category = "UnrealCV")
	UFUNCTION(BlueprintPure, Category = "UnrealCV")
	static int FrameNumber()
	{
		return GFrameNumber;
	}

	UPROPERTY(transient)
	UStaticMesh* CameraMesh;

	// The camera mesh to show visually where the camera is placed
	/** Change the scale of this would not effect the capture component */
	UPROPERTY(transient, EditAnywhere, BlueprintReadWrite, Category = "UnrealCV")
	UStaticMeshComponent* ProxyMeshComponent;

protected:
	/** Provide funtions to convert image format and save file */
	FImageUtil ImageUtil;

	/** Image manipulation on the background, share one worker */
	// static FImageWorker ImageWorker;
	static FImageWorker ImageWorker;

private:

};
