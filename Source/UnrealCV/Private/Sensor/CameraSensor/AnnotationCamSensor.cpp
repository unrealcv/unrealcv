// Weichao Qiu @ 2017
#include "AnnotationCamSensor.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectHash.h"

#include "Component/AnnotationComponent.h"
#include "UnrealcvLog.h"

UAnnotationCamSensor::UAnnotationCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	this->PrimaryComponentTick.bCanEverTick = true;
	this->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	this->ShowFlags.SetMaterials(false); // Check AnnotationComponent.cpp::GetViewRelevance
	this->ShowFlags.SetLighting(false);
	this->ShowFlags.SetPostProcessing(false);
	this->ShowFlags.SetColorGrading(false);
	this->ShowFlags.SetTonemapper(false); // Important to disable this

	this->PostProcessSettings.bOverride_AutoExposureBias = true;
	this->PostProcessSettings.AutoExposureBias = 0; // Overwrite anychange in the scene.
	// Note: The "exposure compensation" in "PostProcessVolume3" in the RR map will destroy the color
	// Note: Saturate the color to 1. This is a mysterious behavior after tedious debug.
	// Note: and the PostProcessing false here seems obvious not enough.

	// Note: the default value of CaptureSource is FinalColorLDR, but the FinalColor might be impact by PostProcessing
	// Note: even though PostProcessing flag is disabled
	// Note: Another option is tuning the PostProcess option flag in the SceneCaptureComponent2D
	// Note: CaptureSource = ESceneCaptureSource::SCS_BaseColor;

	// Note: The generated image is completely dark, which might be caused be the PixelFormat
	// Note: After a lot of trial and error, this is a solution that can work.
}


// this->ShowFlags.SetBSPTriangles(true);
// this->ShowFlags.SetVertexColors(true);
// this->ShowFlags.SetHMDDistortion(false);
// this->ShowFlags.SetTonemapper(false); // This won't take effect here
// GVertexColorViewMode = EVertexColorViewMode::Color;


// Only available for 4.17+
// this->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
// TODO: Write an article about LinearGamma

void UAnnotationCamSensor::InitTextureTarget(int FilmWidth, int FilmHeight)
{
	EPixelFormat PixelFormat = EPixelFormat::PF_B8G8R8A8;
	bool bUseLinearGamma = true;
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, PixelFormat, bUseLinearGamma);
}

void UAnnotationCamSensor::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T)
{
	Super::TickComponent(DeltaTime, TickType, T);
}

void UAnnotationCamSensor::GetAnnotationComponents(UWorld* World, TArray<TWeakObjectPtr<UPrimitiveComponent> >& ComponentList)
{
	if (!IsValid(World))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not get AnnotationComponents, World is invalid"));
		return;
	}

	// Check how much time is spent here!
	TArray<UObject*> UObjectList;
	bool bIncludeDerivedClasses = false;
	EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject;
	EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::AllFlags;
	GetObjectsOfClass(UAnnotationComponent::StaticClass(), UObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);

	for (UObject* Object : UObjectList)
	{
		UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Object);

		if (Component->GetWorld() == World
		&& !ComponentList.Contains(Component))
		{
			ComponentList.Add(Component);
		}
	}

	if (ComponentList.Num() == 0)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("No annotation in the scene to show, fall back to lit mode"));
	}
}

void UAnnotationCamSensor::CaptureSeg(TArray<FColor>& ImageData, int& Width, int& Height)
{
	TArray<TWeakObjectPtr<UPrimitiveComponent> > ComponentList;
	GetAnnotationComponents(this->GetWorld(), ComponentList);

	this->ShowOnlyComponents = ComponentList;

	Capture(ImageData, Width, Height);

	// TODO: Find a faster implementation.
	if (ImageData.Num() != 0)
	{
		for (int i = 0; i < Width * Height; i++)
		{
			ImageData[i].A = 255;
		}
	}
}
