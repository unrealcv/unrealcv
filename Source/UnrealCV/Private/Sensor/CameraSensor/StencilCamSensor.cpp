// Weichao Qiu @ 2017
#include "StencilCamSensor.h"

UStencilCamSensor::UStencilCamSensor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	FString StencilPPMPath = TEXT("Material'/UnrealCV/StencilPPM.StencilPPM'");
	ConstructorHelpers::FObjectFinder<UMaterial> Material(*StencilPPMPath);

	this->StencilPPMaterial= Material.Object;
	SetPostProcessMaterial(StencilPPMaterial);
}

void UStencilCamSensor::SetupRenderTarget()
{
	bool bUseLinearGamma = true;
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
}
