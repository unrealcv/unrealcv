#include "UnrealCVPrivate.h"
#include "ObjMaskCamSensor.h"

UObjMaskCamSensor::UObjMaskCamSensor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	this->ShowFlags.SetMaterials(false);
	this->ShowFlags.SetLighting(false);
	this->ShowFlags.SetBSPTriangles(true);
	this->ShowFlags.SetVertexColors(true);
	this->ShowFlags.SetPostProcessing(false);
	this->ShowFlags.SetHMDDistortion(false);
	this->ShowFlags.SetTonemapper(false); // This won't take effect here

	GVertexColorViewMode = EVertexColorViewMode::Color;
	// this->ShowFlags.SetRendering(false); // This will disable the rendering
}

void UObjMaskCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	bool bUseLinearGamma = false;
	TextureTarget->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->bCaptureEveryFrame = true; // TODO: Check the performance overhead for this
	this->bCaptureOnMovement = false;
}
