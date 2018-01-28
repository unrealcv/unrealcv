#include "UnrealCVPrivate.h"
#include "VertexColorCamSensor.h"
#include "EngineModule.h"
#include "PrimitiveSceneInfo.h"

UVertexColorCamSensor::UVertexColorCamSensor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	this->ShowFlags.SetMaterials(false);
	this->ShowFlags.SetLighting(false);
	this->ShowFlags.SetBSPTriangles(true);
	this->ShowFlags.SetVertexColors(true);
	this->ShowFlags.SetPostProcessing(false);
	this->ShowFlags.SetHMDDistortion(false);
	this->ShowFlags.SetTonemapper(false); // This won't take effect here

	// GVertexColorViewMode = EVertexColorViewMode::Color;
	// this->ShowFlags.SetRendering(false); // This will disable the rendering
}

void UVertexColorCamSensor::OnRegister()
{
	Super::OnRegister();

	TextureTarget = NewObject<UTextureRenderTarget2D>(this);
	// bool bUseLinearGamma = false;
	bool bUseLinearGamma = true; // disable sRGB !
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, EPixelFormat::PF_B8G8R8A8, bUseLinearGamma);
	this->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	this->bCaptureEveryFrame = false;
	this->bCaptureOnMovement = false;
}



	/*
	UWorld* World = GetWorld();
	bool bRequiresHitProxies = false;
	bool bCreateFXSystem = false;
	FSceneInterface* SceneInterface = GetRendererModule().AllocateScene(World, bRequiresHitProxies, bCreateFXSystem, World->FeatureLevel);
	SceneInterface->UpdateSceneCaptureContents(this);

	TArray<FPrimitiveComponentId> ComponentIds = SceneInterface->GetScenePrimitiveComponentIds();
	for (FPrimitiveComponentId Id : ComponentIds)
	{
		FPrimitiveSceneInfo* PrimitiveSceneInfo =  SceneInterface->GetPrimitiveSceneInfo(Id.PrimIDValue);
		FPrimitiveSceneProxy* PrimitiveSceneProxy = PrimitiveSceneInfo->Proxy;
		FStaticMeshSceneProxy* StaticMeshSceneProxy = dynamic_cast<FStaticMeshSceneProxy*>(PrimitiveSceneProxy);
	}

	//if (World && World->Scene && IsVisible())
	//{
	//	// We must push any deferred render state recreations before causing any rendering to happen, to make sure that deleted resource references are updated
	//	World->SendAllEndOfFrameUpdates();
	//	World->Scene->UpdateSceneCaptureContents(this);
	//}
	// this->CaptureScene();
	*/
