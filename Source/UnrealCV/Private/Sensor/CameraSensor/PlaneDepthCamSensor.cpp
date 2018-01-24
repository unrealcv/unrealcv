// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "PlaneDepthCamSensor.h"

UPlaneDepthCamSensor::UPlaneDepthCamSensor(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
	FString PlaneDepthPPMaterialPath = TEXT("Material'/UnrealCV/ScenePlaneDepthWorldUnits.ScenePlaneDepthWorldUnits'");
	ConstructorHelpers::FObjectFinder<UMaterial> Material(*PlaneDepthPPMaterialPath);

	SetPostProcessMaterial(Material.Object);
}

void UPlaneDepthCamSensor::CaptureDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height)
{
	this->CaptureScene();
	Width = this->TextureTarget->SizeX, Height = TextureTarget->SizeY;
	DepthData.AddZeroed(Width * Height);
	FTextureRenderTargetResource* RenderTargetResource = this->TextureTarget->GameThread_GetRenderTargetResource();
	RenderTargetResource->ReadFloat16Pixels(DepthData);
}

void UPlaneDepthCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);

	bool bUseLinearGamma = true;
    // TODO: Check whether InitAutoFormat = Float + UseLinearGamma?
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_FloatRGBA, bUseLinearGamma);

	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	// this->bCaptureEveryFrame = false;
	this->bCaptureEveryFrame = false;
	this->bCaptureOnMovement = false;
}
