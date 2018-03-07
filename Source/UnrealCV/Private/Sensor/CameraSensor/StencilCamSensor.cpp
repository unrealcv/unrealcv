// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "StencilCamSensor.h"

UStencilCamSensor::UStencilCamSensor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	FString StencilPPMPath = TEXT("Material'/UnrealCV/StencilPPM.StencilPPM'");
	ConstructorHelpers::FObjectFinder<UMaterial> Material(*StencilPPMPath);

	this->StencilPPMaterial= Material.Object;
	SetPostProcessMaterial(StencilPPMaterial);
}

void UStencilCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);

	bool bUseLinearGamma = true;
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
	// TextureTarget->InitCustomFormat(Width, Height, EPixelFormat::PF_FloatRGBA, bUseLinearGamma);
	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	// this->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	this->bCaptureEveryFrame = true;
	this->bCaptureOnMovement = false;
}
