// Weichao Qiu @ 2017
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
}

void ULitCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	// bool bUseLinearGamma = false;

	// bool bUseLinearGamma = true;  // true by default
	// bUseLinearGamma requires CaptureEveryFrame!
	bool bUseLinearGamma = false;
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);

	// TextureTarget->InitAutoFormat(Width, Height);
	// TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
	// TextureTarget->TargetGamma = 1;

	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->bCaptureEveryFrame = false; // TODO: Check the performance overhead for this
	this->bCaptureOnMovement = false;

	// ImageWorker.Start();
}
