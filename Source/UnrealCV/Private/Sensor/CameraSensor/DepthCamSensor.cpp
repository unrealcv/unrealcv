// Weichao Qiu @ 2017
#include "DepthCamSensor.h"
#include "AnnotationCamSensor.h"

UDepthCamSensor::UDepthCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	// this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	bIgnoreTransparentObjects = false;
}

void UDepthCamSensor::InitTextureTarget(int FilmWidth, int FilmHeight)
{
	EPixelFormat PixelFormat = EPixelFormat::PF_FloatRGBA;
	bool bUseLinearGamma = true;
 	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_FloatRGBA, bUseLinearGamma);
}

void UDepthCamSensor::CaptureDepth(TArray<float>& DepthData, int& Width, int& Height)
{
	if (!bIgnoreTransparentObjects)
	{
		TArray<TWeakObjectPtr<UPrimitiveComponent> > ComponentList;
		UAnnotationCamSensor::GetAnnotationComponents(this->GetWorld(), ComponentList);
		this->ShowOnlyComponents = ComponentList;
		this->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
		this->ShowFlags.SetMaterials(false); // This will make annotation component visible
	}

	if (!CheckTextureTarget()) return;
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
