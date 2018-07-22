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
 *
 */
UCLASS(abstract)
class UNREALCV_API UBaseCameraSensor : public USceneCaptureComponent2D
{
	GENERATED_BODY()

public:
	UBaseCameraSensor(const FObjectInitializer& ObjectInitializer);

	// virtual void OnRegister() override;

	virtual void SetupRenderTarget();

public:
	// 	FlushRenderingCommands() can make sure the rendering command is finished, but will slow down the game thread
	virtual void CaptureFast(TArray<FColor>& ImageData, int& Width, int& Height);

	/** Save lit to an image file, send the capture command to rendering thread */
	virtual void CaptureToFile(const FString& Filename);

	/** The old version to read TextureBuffer, slow but is sync operation and  correct */
	virtual void Capture(TArray<FColor>& ImageData, int& Width, int& Height);

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

	/** Get the FOV of this camera */
	float GetFOV() { return this->FOVAngle; }
	void SetFOV(float FOV) { this->FOVAngle = FOV; }

	int FilmWidth;
	int FilmHeight;

	/** Get the projection matrix of this camera */
	FString GetProjectionMatrix();

	void SetPostProcessMaterial(UMaterial* PostProcessMaterial);


	/** Similar function to GetCameraView in UCameraComponent, without lockToHMD feature */
	void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

protected:
	EPixelFormat PixelFormat;
	bool bUseLinearGamma;
	/** The TextureBuffer width */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)

	/** The TextureBuffer height */
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)

private:
	/** The camera mesh to show visually where the camera is placed
	Change the scale of this would not effect the capture component */
	// UPROPERTY()
	// UStaticMeshComponent* ProxyMeshComponent;
	// This might not be neccessary
};
