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
	// Should be override by child classes
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	virtual void CaptureFast(TArray<FColor>& ImageData, int& Width, int& Height);

	/** Save lit to an image file, send the capture command to rendering thread */
	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	virtual void CaptureToFile(const FString& Filename);

	/** The old version to read TextureBuffer, slow but is sync operation and  correct */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	virtual void Capture(TArray<FColor>& ImageData, int& Width, int& Height);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	virtual FVector GetSensorLocation()
	{
		return this->GetComponentLocation(); // World space
	}

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	virtual FRotator GetSensorRotation()
	{
		return this->GetComponentRotation(); // World space
	}

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	virtual void SetSensorLocation(FVector Location)
	{
		this->SetWorldLocation(Location);
	}

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	virtual void SetSensorRotation(FRotator Rotator)
	{
		this->SetWorldRotation(Rotator);
	}

	/** Get the FOV of this camera */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	float GetSensorFOV() { return this->FOVAngle; }

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetFOV(float FOV) { this->FOVAngle = FOV; }

	/** Get the projection matrix of this camera */
	FString GetProjectionMatrix();

	void SetPostProcessMaterial(UMaterial* PostProcessMaterial);

	/** The TextureBuffer width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unrealcv")
	int FilmWidth;

	/** The TextureBuffer height */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unrealcv")
	int FilmHeight;

	UPROPERTY(transient)
	UStaticMesh* CameraMesh;

	// The camera mesh to show visually where the camera is placed
	/** Change the scale of this would not effect the capture component */
	UPROPERTY(transient, EditAnywhere, BlueprintReadWrite, Category = "unrealcv")
	UStaticMeshComponent* ProxyMeshComponent;

	// Similar function to GetCameraView in UCameraComponent, without lockToHMD feature
	void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

protected:
	/** Provide funtions to convert image format and save file */
	FImageUtil ImageUtil;

	/** Image manipulation on the background, share one worker */
	// static FImageWorker ImageWorker;
	static FImageWorker ImageWorker;

private:

};
