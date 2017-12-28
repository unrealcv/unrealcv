// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "DepthCamSensor.h"

UDepthCamSensor::UDepthCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{

}

void UDepthCamSensor::CaptureDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height)
{
	this->CaptureScene();
	Width = this->TextureTarget->SizeX, Height = TextureTarget->SizeY;
	DepthData.AddZeroed(Width * Height);
	FTextureRenderTargetResource* RenderTargetResource = this->TextureTarget->GameThread_GetRenderTargetResource();
	RenderTargetResource->ReadFloat16Pixels(DepthData);
}

void UDepthCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);

	bool bUseLinearGamma = true;
	// TextureTarget->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
	TextureTarget->InitCustomFormat(Width, Height, EPixelFormat::PF_FloatRGBA, bUseLinearGamma);

	// this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	this->bCaptureEveryFrame = false; // true by default
	this->bCaptureOnMovement = false;
}
