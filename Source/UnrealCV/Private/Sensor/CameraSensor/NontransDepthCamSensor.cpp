// Weichao Qiu @ 2018
#include "UnrealCVPrivate.h"
#include "NontransDepthCamSensor.h"

void GetAnnotationComponents(UWorld* World, TArray<TWeakObjectPtr<UPrimitiveComponent> >& ComponentList);

UNontransDepthCamSensor::UNontransDepthCamSensor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	this->ShowFlags.SetMaterials(false);
	this->ShowFlags.SetLighting(false);
	this->ShowFlags.SetPostProcessing(false);
}

void UNontransDepthCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);

	bool bUseLinearGamma = true;
	// TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_FloatRGBA, bUseLinearGamma);
	// this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	this->bCaptureEveryFrame = false;
	this->bCaptureOnMovement = false;
}

void UNontransDepthCamSensor::CaptureDepth(TArray<float>& DepthData, int& Width, int& Height)
{
	TArray<TWeakObjectPtr<UPrimitiveComponent> > ComponentList;
	GetAnnotationComponents(this->GetWorld(), ComponentList);

	this->ShowOnlyComponents = ComponentList;

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
