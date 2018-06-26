// Weichao Qiu @ 2017
#pragma once

#include "Runtime/Engine/Classes/Components/PrimitiveComponent.h"
#include "FusionCamSensor.generated.h"

// class UNREALCV_API UFusionCamSensor : public UBaseCameraSensor
UCLASS(meta = (BlueprintSpawnableComponent))
class UNREALCV_API UFusionCamSensor : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	UFusionCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void OnRegister() override;

	virtual bool GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut);

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class ULitCamSensor* LitCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UDepthCamSensor* DepthCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UVertexColorCamSensor* VertexColorCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UNormalCamSensor* NormalCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UStencilCamSensor* StencilCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UAnnotationCamSensor* AnnotationCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UPlaneDepthCamSensor* PlaneDepthCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UVisDepthCamSensor* VisDepthCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UNontransDepthCamSensor* NontransDepthCamSensor;

	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class ULitSlowCamSensor* LitSlowCamSensor;

	/** This preview camera is used for UE version < 4.17 which only support UCameraComponent PIP preview
	See the difference between
	https://github.com/EpicGames/UnrealEngine/blob/4.17/Engine/Source/Editor/LevelEditor/Private/SLevelViewport.cpp#L3927
	and
	https://github.com/EpicGames/UnrealEngine/blob/4.16/Engine/Source/Editor/LevelEditor/Private/SLevelViewport.cpp#L3908
	*/
	UPROPERTY(Instanced, VisibleAnywhere, BlueprintReadOnly)
	class UCameraComponent* PreviewCamera;

	/** Get rgb data */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetLit(TArray<FColor>& LitData, int& InOutWidth, int& InOutHeight);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetLitSlow(TArray<FColor>& LitData, int& InOutWidth, int& InOutHeight);

	/** Get depth data */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetDepth(TArray<float>& DepthData, int& InOutWidth, int& InOutHeight);
	void GetPlaneDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height);
	void GetVisDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height);

	/** Get surface normal data */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetNormal(TArray<FColor>& NormalData, int& Width, int& Height);

	/** Get object mask data, the annotation color can be extracted from FObjectAnnotator */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetObjectMask(TArray<FColor>& ObjMaskData, int& Width, int& Height);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetVertexColor(TArray<FColor>& VertexColorData, int& Width, int& Height);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetStencil(TArray<FColor>& StencilData, int& Width, int& Height);

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
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	FVector GetSensorLocation();

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	FRotator GetSensorRotation();

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetSensorLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetSensorRotation(FRotator Rotator);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	float GetFilmHeight();

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	float GetFilmWidth();

private:
	UPROPERTY()
	TArray<class UBaseCameraSensor*> FusionSensors;
};
