// Weichao Qiu @ 2017
// Should not be used in blueprint
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

#include "BaseCameraSensor.generated.h"

/**
 * A base camera sensor for ground truth capture
 */
UCLASS(abstract)
class UNREALCV_API UBaseCameraSensor : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	UBaseCameraSensor(const FObjectInitializer& ObjectInitializer);

	// 	FlushRenderingCommands() can make sure the rendering command is finished, but will slow down the game thread
	void CaptureFast(TArray<FColor>& ImageData, int& Width, int& Height);

	/** Save lit to an image file, send the capture command to rendering thread */
	void CaptureToFile(const FString& Filename);

	/** The old version to read TextureBuffer, slow but is sync operation and  correct */
	void Capture(TArray<FColor>& ImageData, int& Width, int& Height);

	/** Get/set the sensor location / rotation */
	FVector GetSensorLocation()
	{
		return this->GetComponentLocation(); // World space
	}
	void SetSensorLocation(FVector Location)
	{
		this->SetWorldLocation(Location);
	}

	FRotator GetSensorRotation()
	{
		return this->GetComponentRotation(); // World space
	}
	void SetSensorRotation(FRotator Rotator)
	{
		this->SetWorldRotation(Rotator);
	}

	/** Get/set the FOV of this camera */
	float GetFOV() { return this->FOVAngle; }
	void SetFOV(float FOV) { this->FOVAngle = FOV; }

	/** Get/set the sensor film size */
	void SetFilmSize(int Width, int Height);
	int GetFilmWidth();
	int GetFilmHeight();

	/** Get the projection matrix of this camera */
	FString GetProjectionMatrix();

	virtual void InitTextureTarget(int FilmWidth, int FilmHeight);

	void SetPostProcessMaterial(UMaterial* PostProcessMaterial);

	/** Similar function to GetCameraView in UCameraComponent, without lockToHMD feature */
	void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

protected:
	/** Check whether the TextureTarget is correctly initialized */
	bool CheckTextureTarget();

	int FilmWidth;

	int FilmHeight;
};
