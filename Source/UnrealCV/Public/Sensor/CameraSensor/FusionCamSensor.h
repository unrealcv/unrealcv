// Weichao Qiu @ 2017
#pragma once

#include "Runtime/Engine/Classes/Camera/CameraTypes.h"
#include "Runtime/Engine/Classes/Components/PrimitiveComponent.h"
#include "FusionCamSensor.generated.h"

UENUM(BlueprintType)
enum class ELitMode : uint8
{
	Lit,
	Slow
};

UENUM(BlueprintType)
enum class EDepthMode : uint8
{
	PlaneDepth,
	DistToCamCenter
};

UENUM(BlueprintType)
enum class ESegMode : uint8
{
	AnnotationComponent,
	VertexColor,
	CustomStencil
};

UENUM(BlueprintType)
enum class EPresetFilmSize : uint8
{
	F640x480,
	F720p,
	F1080p
};

UCLASS(meta = (BlueprintSpawnableComponent))
class UNREALCV_API UFusionCamSensor : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	UFusionCamSensor(const FObjectInitializer& ObjectInitializer);

	// virtual void OnRegister() override;
	virtual bool GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut);

	/** Get rgb data */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetLit(TArray<FColor>& LitData, int& InOutWidth, int& InOutHeight, ELitMode LitMode = ELitMode::Lit);

	/** Get depth data */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetDepth(TArray<float>& DepthData, int& InOutWidth, int& InOutHeight, EDepthMode DepthMode = EDepthMode::PlaneDepth);

	/** Get surface normal data */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetNormal(TArray<FColor>& NormalData, int& Width, int& Height);

	/** Get object mask data, the annotation color can be extracted from FObjectAnnotator */
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetSeg(TArray<FColor>& ObjMaskData, int& Width, int& Height, ESegMode SegMode = ESegMode::AnnotationComponent);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	FVector GetSensorLocation();

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	FRotator GetSensorRotation();

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetSensorLocation(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetSensorRotation(FRotator Rotator);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetFilmSize(int Width, int Height);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	float GetFilmWidth() { return FilmWidth; }

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	float GetFilmHeight() { return FilmHeight; }

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	float GetSensorFOV();

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetSensorFOV(float FOV);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetProjectionType(ECameraProjectionMode::Type ProjectionType);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	void SetOrthoWidth(float OrthoWidth);

	// UFUNCTION(BlueprintPure, Category = "unrealcv")
	// float GetFilmHeight();

	// UFUNCTION(BlueprintPure, Category = "unrealcv")
	// float GetFilmWidth();

	// void SetFilmSize(int Width, int Height);

	UPROPERTY(meta = (AllowPrivateAccess= "true"))
	class UCameraComponent* PreviewCamera;

	virtual void BeginPlay() override;


	static TArray<UFusionCamSensor*> GetComponents(AActor* Actor);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent) override;
#endif

private:
	UPROPERTY(EditInstanceOnly, meta=(AllowPrivateAccess = "true"), Category = "unrealcv")
	EPresetFilmSize PresetFilmSize;

	UPROPERTY(EditInstanceOnly, meta=(AllowPrivateAccess = "true"), Category = "unrealcv")
	int FilmWidth;

	UPROPERTY(EditInstanceOnly, meta=(AllowPrivateAccess = "true"), Category = "unrealcv")
	int FilmHeight;

	UPROPERTY(EditInstanceOnly, meta=(AllowPrivateAccess = "true"), Category = "unrealcv")
	float FOV;

protected:
	UPROPERTY()
	TArray<class UBaseCameraSensor*> FusionSensors;

	UPROPERTY(EditDefaultsOnly, Category = "unrealcv")
	class UDepthCamSensor* DepthCamSensor;

	UPROPERTY(EditDefaultsOnly, Category = "unrealcv")
	class UNormalCamSensor* NormalCamSensor;

	UPROPERTY(EditDefaultsOnly, Category = "unrealcv")
	class UAnnotationCamSensor* AnnotationCamSensor;

	UPROPERTY(EditDefaultsOnly, Category = "unrealcv")
	class ULitCamSensor* LitCamSensor;

	/** This preview camera is used for UE version < 4.17 which only support UCameraComponent PIP preview
	See the difference between
	https://github.com/EpicGames/UnrealEngine/blob/4.17/Engine/Source/Editor/LevelEditor/Private/SLevelViewport.cpp#L3927
	and
	https://github.com/EpicGames/UnrealEngine/blob/4.16/Engine/Source/Editor/LevelEditor/Private/SLevelViewport.cpp#L3908
	*/


};
