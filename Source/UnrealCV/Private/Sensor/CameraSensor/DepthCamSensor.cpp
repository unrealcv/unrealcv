// Weichao Qiu @ 2017
#include "DepthCamSensor.h"

UDepthCamSensor::UDepthCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;

	PixelFormat = EPixelFormat::PF_FloatRGBA;
	bUseLinearGamma = true;
}

// void UDepthCamSensor::SetupRenderTarget()
// {
// 	bool bUseLinearGamma = true;
// 	// TextureTarget->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
// 	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_FloatRGBA, bUseLinearGamma);
// }

void UDepthCamSensor::CaptureDepth(TArray<float>& DepthData, int& Width, int& Height)
{
	this->SetupRenderTarget();
	this->CaptureScene();
	Width = this->TextureTarget->SizeX, Height = TextureTarget->SizeY;
	DepthData.AddZeroed(Width * Height); // or AddUninitialized(FloatColorDepthData.Num());
	FTextureRenderTargetResource* RenderTargetResource = this->TextureTarget->GameThread_GetRenderTargetResource();
	TArray<FFloat16Color> FloatColorDepthData;
	RenderTargetResource->ReadFloat16Pixels(FloatColorDepthData);

	// TODO: Find a faster way to chunk a channel
	for (int i = 0; i < FloatColorDepthData.Num(); i++)
	{
		FFloat16Color& FloatColor = FloatColorDepthData[i];
		DepthData[i] = FloatColor.R;
	}
}
