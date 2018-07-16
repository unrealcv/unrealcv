// Weichao Qiu @ 2017
#include "LitCamSensor.h"

ULitCamSensor::ULitCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
}

void ULitCamSensor::SetupRenderTarget()
{
	bool bUseLinearGamma = false;
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
}

// bool bUseLinearGamma = false;
// bool bUseLinearGamma = true;  // true by default
// bUseLinearGamma requires CaptureEveryFrame!
// TextureTarget->InitAutoFormat(Width, Height);
// TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
// TextureTarget->TargetGamma = 1;

