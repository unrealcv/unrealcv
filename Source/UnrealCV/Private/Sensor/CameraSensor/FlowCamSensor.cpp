// shc @ 2025
#include "FlowCamSensor.h"

UFlowCamSensor::UFlowCamSensor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	FString OpticalFlowPPMaterialPath = TEXT("Material'/UnrealCV/OpticalFlowMaterial.OpticalFlowMaterial'");
	// FString OpticalFlowPPMaterialPath = TEXT("Material'/UnrealCV/WorldNormal.WorldNormal'");

	// Assertion failed: IsInGameThread() [File:D:\build\++UE5\Sync\Engine\Source\Runtime\CoreUObject\Private\Serialization\AsyncLoading.cpp] [Line: 7453] 
	// LoadPackageAsync is only thread-safe when using the zenloader (i.e. AsyncLoading2).
	// This error may occur if UnrealCV has been updated, but the project hasn't been fully rebuilt.
	// To resolve this, try loading the material in the BeginPlay method.
	// Note: A similar FObjectFinder is used in the constructor of NormalCamSensor.
	// To fix this issue, try rebuilding the project completely.
	ConstructorHelpers::FObjectFinder<UMaterial> Material(*OpticalFlowPPMaterialPath);

	if (Material.Object)
	{
		OpticalFlowPPMaterial = Material.Object;
		// SetPostProcessMaterial(OpticalFlowPPMaterial);
		UMaterialInstanceDynamic* PostProcessMaterialInstance = UMaterialInstanceDynamic::Create(OpticalFlowPPMaterial, nullptr);
		if (!PostProcessMaterialInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Could not create the material instance dynamic"), *FString(__FUNCTION__))
		} else {
			PostProcessMaterialInstance->SetScalarParameterValue(TEXT("OpticalFlowScale"), 30.0);
			PostProcessSettings.WeightedBlendables.Array.Empty();
			PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, PostProcessMaterialInstance));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("OpticalFlowMaterial not found at %s"), *OpticalFlowPPMaterialPath);
	}
}

void UFlowCamSensor::SetFilmSize(int Width, int Height)
{
	Super::SetFilmSize(Width, Height);
	if (OpticalFlowPPMaterial)
	{
		UMaterialInstanceDynamic* PostProcessMaterialInstance = UMaterialInstanceDynamic::Create(OpticalFlowPPMaterial, nullptr);
		if (!PostProcessMaterialInstance)
		{
			UE_LOG(LogTemp, Error, TEXT("%s: Could not create the material instance dynamic"), *FString(__FUNCTION__))
		} else {
			PostProcessMaterialInstance->SetScalarParameterValue(TEXT("OpticalFlowScale"), 30.0);
			PostProcessSettings.WeightedBlendables.Array.Empty();
			PostProcessSettings.WeightedBlendables.Array.Add(FWeightedBlendable(1.0f, PostProcessMaterialInstance));
		}
	}
}

// void UFlowCamSensor::InitTextureTarget(int filmWidth, int filmHeight)
// {
// 	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
// 	TextureTarget->InitAutoFormat(filmWidth, filmHeight);
// 	TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
// }

// void UFlowCamSensor::CaptureFlow(TArray<FColor>& Image, int& Width, int& Height)
// {
// 	SCOPE_CYCLE_COUNTER(STAT_CaptureFlow);
// 	if (!CheckTextureTarget())
// 	{
// 		InitTextureTarget(this->FilmWidth, this->FilmHeight);
// 		if (!CheckTextureTarget())
// 		{
// 			UE_LOG(LogUnrealCV, Error, TEXT("Failed to initialize TextureTarget."));
// 			return;
// 		}
// 	}
// 	this->CaptureScene();
// 	FReadSurfaceDataFlags ReadSurfaceDataFlags;
// 	ReadSurfaceDataFlags.SetLinearToGamma(false);
// 	TextureTarget->GameThread_GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
// 	if (Image.Num() == 0)
// 	{
// 		UE_LOG(LogUnrealCV, Warning, TEXT("Captured optical flow data is empty."));
// 	}
// 	Width = GetFilmWidth();
// 	Height = GetFilmHeight();
// }