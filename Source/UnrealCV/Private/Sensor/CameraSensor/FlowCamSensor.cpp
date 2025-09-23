// Weichao Qiu @ 2017
#include "FlowCamSensor.h"
#include "UnrealcvLog.h"
#include "UnrealcvStats.h"

#include "Runtime/Engine/Classes/Engine/Engine.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "TextureResource.h"

DECLARE_CYCLE_STAT(TEXT("UFlowCamSensor::CaptureFlow"), STAT_CaptureFlow, STATGROUP_UnrealCV);

UFlowCamSensor::UFlowCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	FString OpticalFlowPPMaterialPath = TEXT("Material'/UnrealCV/OpticalFlowMaterial.OpticalFlowMaterial'");
	ConstructorHelpers::FObjectFinder<UMaterial> Material(*OpticalFlowPPMaterialPath);

	if (Material.Object != NULL)
	{
		SetPostProcessMaterial(Material.Object);
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("OpticalFlowMaterial not found at %s"), *OpticalFlowPPMaterialPath);
	}
}

void UFlowCamSensor::InitTextureTarget(int filmWidth, int filmHeight)
{
	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	TextureTarget->InitAutoFormat(filmWidth, filmHeight);
	TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
}

void UFlowCamSensor::CaptureFlow(TArray<FColor>& Image, int& Width, int& Height)
{
	SCOPE_CYCLE_COUNTER(STAT_CaptureFlow);
	if (!CheckTextureTarget())
	{
		InitTextureTarget(this->FilmWidth, this->FilmHeight);
		if (!CheckTextureTarget())
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Failed to initialize TextureTarget."));
			return;
		}
	}
	this->CaptureScene();
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false);
	TextureTarget->GameThread_GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	if (Image.Num() == 0)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Captured optical flow data is empty."));
	}
	Width = GetFilmWidth();
	Height = GetFilmHeight();
}