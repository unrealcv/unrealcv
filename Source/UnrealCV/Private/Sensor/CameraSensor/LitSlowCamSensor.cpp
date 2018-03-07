// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "LitSlowCamSensor.h"

ULitSlowCamSensor::ULitSlowCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	SetFOV(90);
}

void ULitSlowCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	TextureTarget->InitAutoFormat(FilmWidth, FilmHeight);
	TextureTarget->TargetGamma = GEngine->GetDisplayGamma();

	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->bCaptureEveryFrame = true; 
	this->bCaptureOnMovement = false;
}

void ULitSlowCamSensor::Capture(TArray<FColor>& Image, int& Width, int& Height)
{
	this->CaptureScene();
	FReadSurfaceDataFlags ReadSurfaceDataFlags;
	ReadSurfaceDataFlags.SetLinearToGamma(false); 
	// TextureTarget->GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	TextureTarget->GameThread_GetRenderTargetResource()->ReadPixels(Image, ReadSurfaceDataFlags);
	Width = FilmWidth;
	Height = FilmHeight;
}