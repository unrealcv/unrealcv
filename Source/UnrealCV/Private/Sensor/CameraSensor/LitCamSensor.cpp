#include "UnrealCVPrivate.h"
#include "LitCamSensor.h"


ULitCamSensor::ULitCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// FString LitPPMaterialPath = TEXT("Material'/ue4_human/PostProcessMaterial/LitPPM.LitPPM'");
	// ConstructorHelpers::FObjectFinder<UMaterial> Material(*LitPPMaterialPath);
    //
	// LitPPMaterial = Material.Object;
	// // ConsturctorHelpers is only available for UObject.
	// SetPostProcessMaterial(LitPPMaterial);

	SetFOV(90);

	Width = 640;
	Height = 480;
}

void ULitCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	bool bUseLinearGamma = false;
	TextureTarget->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->bCaptureEveryFrame = true; // TODO: Check the performance overhead for this
	this->bCaptureOnMovement = false;

	// ImageWorker.Start();
}
