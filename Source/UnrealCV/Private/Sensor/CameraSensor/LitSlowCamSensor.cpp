// Weichao Qiu @ 2017
#include "LitSlowCamSensor.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

ULitSlowCamSensor::ULitSlowCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void ULitSlowCamSensor::SetupRenderTarget()
{
	if (!IsValid(TextureTarget))
	{
		TextureTarget = NewObject<UTextureRenderTarget2D>(this); 
	}

	if (TextureTarget->SizeX != FilmWidth || TextureTarget->SizeY != FilmHeight) 
	{
		TextureTarget->InitAutoFormat(FilmWidth, FilmHeight);
		TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
	}
}

void ULitSlowCamSensor::Capture(TArray<FColor>& Image, int& Width, int& Height)
{
	this->SetupRenderTarget();
	this->CaptureScene();
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false); 
	// TextureTarget->GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	TextureTarget->GameThread_GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	Width = FilmWidth;
	Height = FilmHeight;
}