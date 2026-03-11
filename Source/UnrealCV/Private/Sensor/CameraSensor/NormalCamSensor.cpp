// Weichao Qiu @ 2018
#include "NormalCamSensor.h"
#include "UnrealcvLog.h"

UNormalCamSensor::UNormalCamSensor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	this->ShowFlags.SetPostProcessing(true);
	this->ShowFlags.SetPostProcessMaterial(true);

	FString NormalPPMaterialPath = TEXT("Material'/UnrealCV/WorldNormal.WorldNormal'");
	ConstructorHelpers::FObjectFinder<UMaterial> Material(*NormalPPMaterialPath);
	if (IsValid(Material.Object))
	{
		this->SurfaceNormalPPMaterial = Material.Object;
		SetPostProcessMaterial(SurfaceNormalPPMaterial);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load normal post process material: %s"), *NormalPPMaterialPath);
	}
}

// void UNormalCamSensor::SetupRenderTarget()
// {
// 	bool bUseLinearGamma = false;
// 	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
// }

void UNormalCamSensor::InitTextureTarget(int filmWidth, int filmHeight)
{
	Super::InitTextureTarget(filmWidth, filmHeight);
	if (this->SurfaceNormalPPMaterial)
	{
		SetPostProcessMaterial(this->SurfaceNormalPPMaterial);
	}
}

void UNormalCamSensor::CaptureNormal(TArray<FColor>& ImageData, int& Width, int& Height)
{
	if (!CheckTextureTarget())
	{
		InitTextureTarget(this->FilmWidth, this->FilmHeight);
		if (!CheckTextureTarget())
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Failed to initialize TextureTarget."));
			return;
		}
	}
	Capture(ImageData, Width, Height);
	// if (ImageData.Num() != 0)
	// {
	// 	if (Width > 0 && Height > 0 && static_cast<uint32>(Width * Height) == ImageData.Num())
	// 	{
	// 		SetAlpha(ImageData);
	// 	}
	// 	else
	// 	{
	// 		UE_LOG(LogUnrealCV, Warning, TEXT("Invalid Width or Height for ImageData in CaptureNormal"));
	// 	}
	// }
}
