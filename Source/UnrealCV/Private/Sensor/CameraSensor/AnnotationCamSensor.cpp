// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "AnnotationCamSensor.h"

UAnnotationCamSensor::UAnnotationCamSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	this->PrimaryComponentTick.bCanEverTick = true;
	this->ShowFlags.SetMaterials(false); // Check AnnotationComponent.cpp::GetViewRelevance
	this->ShowFlags.SetLighting(false);
	this->ShowFlags.SetPostProcessing(false);

	// this->ShowFlags.SetBSPTriangles(true);
	// this->ShowFlags.SetVertexColors(true);
	// this->ShowFlags.SetHMDDistortion(false);
	// this->ShowFlags.SetTonemapper(false); // This won't take effect here
	// GVertexColorViewMode = EVertexColorViewMode::Color;
}

void UAnnotationCamSensor::OnRegister()
{
	Super::OnRegister();

	// Only available for 4.17+
	// this->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	// TODO: Write an article about LinearGamma
	bool bUseLinearGamma = true;
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->bCaptureEveryFrame = false;
	this->bCaptureOnMovement = false;
}

void UAnnotationCamSensor::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T)
{
	Super::TickComponent(DeltaTime, TickType, T);

}

void GetAnnotationComponents(UWorld* World, TArray<TWeakObjectPtr<UPrimitiveComponent> >& ComponentList)
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

void UAnnotationCamSensor::Capture(TArray<FColor>& ImageData, int& Width, int& Height)
{
	TArray<TWeakObjectPtr<UPrimitiveComponent> > ComponentList;
	GetAnnotationComponents(this->GetWorld(), ComponentList);

	this->ShowOnlyComponents = ComponentList;

	Super::Capture(ImageData, Width, Height);
}
