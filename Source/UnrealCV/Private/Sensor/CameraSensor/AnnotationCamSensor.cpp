// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "AnnotationCamSensor.h"

UAnnotationCamSensor::UAnnotationCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	this->PrimaryComponentTick.bCanEverTick = true;
	this->ShowFlags.SetMaterials(false);
	this->ShowFlags.SetLighting(false);
	this->ShowFlags.SetPostProcessing(false);
	// this->ShowFlags.SetBSPTriangles(true);
	// this->ShowFlags.SetVertexColors(true);
	// this->ShowFlags.SetHMDDistortion(false);
	// this->ShowFlags.SetTonemapper(false); // This won't take effect here
    //
	// GVertexColorViewMode = EVertexColorViewMode::Color;
}

void UAnnotationCamSensor::OnRegister()
{
	Super::OnRegister();

	// this->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	bool bUseLinearGamma = true; // This will disable sRGB
	TextureTarget->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->bCaptureEveryFrame = true; // TODO: Check the performance overhead for this
	this->bCaptureOnMovement = false;

}


void UAnnotationCamSensor::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T)
{
	Super::TickComponent(DeltaTime, TickType, T);

	// Check how much time is spent here!
	TArray<UObject*> UObjectList;
	bool bIncludeDerivedClasses = false;
	EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject;
	EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::AllFlags;
	GetObjectsOfClass(UAnnotationComponent::StaticClass(), UObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);

	TArray<UAnnotationComponent*> ComponentList;
	for (UObject* Object : UObjectList)
	{
		UPrimitiveComponent* Component = Cast<UPrimitiveComponent>(Object);
		if (Component->GetWorld() == GetWorld()
		&& !ShowOnlyComponents.Contains(Component))
		{
			this->ShowOnlyComponents.Add(Component);
		}
	}
	if (this->ShowOnlyComponents.Num() == 0)
	{
		ScreenLog("No annotation in the scene to show, fall back to lit mode");
	}
}
