// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "FusionCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UFusionCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()
public:

	UFusionCamSensor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class ULitCamSensor* LitCamSensor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UDepthCamSensor* DepthCamSensor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UVertexColorCamSensor* ObjMaskCamSensor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UNormalCamSensor* NormalCamSensor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UStencilCamSensor* StencilCamSensor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UAnnotationCamSensor* AnnotationCamSensor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UPlaneDepthCamSensor* PlaneDepthCamSensor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UVisDepthCamSensor* VisDepthCamSensor;

	void GetLit(TArray<FColor>& LitData, int& Width, int& Height);

	void GetDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height);
	void GetPlaneDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height);
	void GetVisDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height);

	void GetNormal(TArray<FColor>& NormalData, int& Width, int& Height);

	void GetObjectMask(TArray<FColor>& ObjMaskData, int& Width, int& Height);
	void GetVertexColor(TArray<FColor>& VertexColorData, int& Width, int& Height);
	void GetStencil(TArray<FColor>& StencilData, int& Width, int& Height);

	virtual void OnRegister() override;

	// void GetLitFilename(const FString& Filename);
	// void GetLitPng(TArray<uint8>& PngBinaryData);

	/** Save depth file as exr files */
	// void GetDepthFilename(const FString& Filename);
	// void GetDepthNpy(TArray<uint8>& NpyBinaryData);

	// void GetObjectMaskFilename(const FString& Filename);
	// void GetObjectMaskNpy(TArray<uint8>& NpyBinaryData);
	// void GetObjectMaskPng(TArray<uint8>& PngBinaryData);

	// void GetNormalFilename(const FString& Filename);
	// void GetNormalNpy(TArray<uint8>& NpyBinaryData);


private:
	FImageUtil ImageUtil;

	UPROPERTY()
	TArray<UBaseCameraSensor*> FusionSensors;
};
