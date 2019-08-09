// Weichao Qiu @ 2017
#include "LitCamSensor.h"
#include "UnrealcvLog.h"
#include "UnrealcvStats.h"

#include "Runtime/Engine/Classes/Engine/Engine.h"


DECLARE_CYCLE_STAT(TEXT("ULitCamSensor::CaptureLit"), STAT_CaptureLit, STATGROUP_UnrealCV);

ULitCamSensor::ULitCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

// void ULitCamSensor::SetupRenderTarget()
// {
// 	bool bUseLinearGamma = false;
// 	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
// }

// bool bUseLinearGamma = false;
// bool bUseLinearGamma = true;  // true by default
// bUseLinearGamma requires CaptureEveryFrame!
// TextureTarget->InitAutoFormat(Width, Height);
// TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
// TextureTarget->TargetGamma = 1;

void ULitCamSensor::InitTextureTarget(int FilmWidth, int FilmHeight)
{
	TextureTarget = NewObject<UTextureRenderTarget2D>(this); 
	TextureTarget->InitAutoFormat(FilmWidth, FilmHeight);
	TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
}

void ULitCamSensor::CaptureLit(TArray<FColor>& Image, int& Width, int& Height)
{
	SCOPE_CYCLE_COUNTER(STAT_CaptureLit);
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
	// TextureTarget->GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	TextureTarget->GameThread_GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	if (Image.Num() == 0)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Captured lit data is empty."));
	}
	Width = GetFilmWidth();
	Height = GetFilmHeight();
}