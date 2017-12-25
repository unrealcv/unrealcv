#include "UnrealCVPrivate.h"
#include "AnnotationComponent.h"

class FAnnotationSceneProxy : public FStaticMeshSceneProxy
{
	/*static void OverrideMaterial(FStaticMeshSceneProxy* StaticMeshSceneProxy)
	{
		StaticMeshSceneProxy->LODs;
	}*/

public:
	FAnnotationSceneProxy(UStaticMeshComponent* Component, bool bForceLODsShareStaticLighting, UMaterialInterface* AnnotationMaterial) :
		FStaticMeshSceneProxy(Component, bForceLODsShareStaticLighting)
	{
		this->bVerifyUsedMaterials = false;
		// From StaticMeshRenderer
		int32 NumLODs = RenderData->LODResources.Num();
		for(int32 LODIndex = ClampedMinLOD; LODIndex < NumLODs; LODIndex++)
		{
			const FStaticMeshLODResources& LODModel = RenderData->LODResources[LODIndex];
			FLODInfo& ProxyLODInfo = LODs[LODIndex];
			for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
			{
				// const FMaterial* Material = ProxyLODInfo.Sections[SectionIndex].Material->GetRenderProxy(false)->GetMaterial(FeatureLevel);
				// ProxyLODInfo.Sections[SectionIndex].Material = GEngine->VertexColorViewModeMaterial_ColorOnly;
				// ProxyLODInfo.Sections[SectionIndex].Material = UMaterial::GetDefaultMaterial(MD_Surface);
				ProxyLODInfo.Sections[SectionIndex].Material = AnnotationMaterial;
			}
		}
		bCastShadow = false;
		// this->LODs.
	}

	//virtual void GetDynamicMeshElements(
	//	const TArray < const FSceneView * > & Views,
	//	const FSceneViewFamily & ViewFamily,
	//	uint32 VisibilityMap,
	//	FMeshElementCollector & Collector) override;
	virtual void GetDynamicMeshElements(
		const TArray < const FSceneView * > & Views,
		const FSceneViewFamily & ViewFamily,
		uint32 VisibilityMap,
		FMeshElementCollector & Collector) const override;

	virtual bool GetMeshElement
	(
		int32 LODIndex,
		int32 BatchIndex,
		int32 ElementIndex,
		uint8 InDepthPriorityGroup,
		bool bUseSelectedMaterial,
		bool bUseHoveredMaterial,
		bool bAllowPreCulledIndices,
		FMeshBatch & OutMeshBatch
	) const override;

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;
};

FPrimitiveViewRelevance FAnnotationSceneProxy::GetViewRelevance(const FSceneView * View) const
{
	// View->Family->EngineShowFlags.
	FPrimitiveViewRelevance ViewRelevance;
	ViewRelevance.bDrawRelevance = 0;
	if (!View->Family->EngineShowFlags.Materials)
	{
		return FStaticMeshSceneProxy::GetViewRelevance(View);
	}
	else
	{
		return ViewRelevance;
	}
}


void FAnnotationSceneProxy::GetDynamicMeshElements(
	const TArray < const FSceneView * > & Views,
	const FSceneViewFamily & ViewFamily,
	uint32 VisibilityMap,
	FMeshElementCollector & Collector) const
{
	FStaticMeshSceneProxy::GetDynamicMeshElements(Views, ViewFamily, VisibilityMap, Collector);
	//if (!ViewFamily.EngineShowFlags.Materials) // Only render this if material is disabled
	//// Check the EngineFlags to see whether to show annotation
	//{
	//	FStaticMeshSceneProxy::GetDynamicMeshElements(Views, ViewFamily, VisibilityMap, Collector);
	//}
}

bool FAnnotationSceneProxy::GetMeshElement(
	int32 LODIndex,
	int32 BatchIndex,
	int32 ElementIndex,
	uint8 InDepthPriorityGroup,
	bool bUseSelectedMaterial,
	bool bUseHoveredMaterial,
	bool bAllowPreCulledIndices,
	FMeshBatch & OutMeshBatch) const
{
	return FStaticMeshSceneProxy::GetMeshElement(LODIndex, BatchIndex, ElementIndex, InDepthPriorityGroup,
		bUseSelectedMaterial, bUseHoveredMaterial, bAllowPreCulledIndices, OutMeshBatch);
}

UAnnotationComponent::UAnnotationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FString MaterialPath = TEXT("Material'/UnrealCV/AnnotationColor.AnnotationColor'");
	// FString MeterialPath = TEXT("MaterialInstanceConstant'/UnrealCV/AnnotationColor_Inst.AnnotationColor_Inst'");
	// static ConstructorHelpers::FObjectFinder<UMaterialInstanceDynamic> AnnotationMaterialObject(*MaterialPath);
	static ConstructorHelpers::FObjectFinder<UMaterial> AnnotationMaterialObject(*MaterialPath);
	// UMaterialInstanceDynamic* AnnotationMaterial = AnnotationMaterialObject.Object;
	AnnotationMaterial = AnnotationMaterialObject.Object;
}

FPrimitiveSceneProxy* UAnnotationComponent::CreateSceneProxy()
{
	// This can not be placed in the constructor, MID, material instance dynamic
	if (AnnotationMID == nullptr)
	{
		AnnotationMID = UMaterialInstanceDynamic::Create(AnnotationMaterial, this);
	}
	// UMaterialInstanceDynamic* AnnotationMID = UMaterialInstanceDynamic::Create(AnnotationMaterial, this);
	// FColor AnnotationColor = FColor::MakeRandomColor();
	// FLinearColor AnnotationColor = FLinearColor::MakeRandomColor();
	// AnnotationMID->SetVectorParameterByIndex(0, AnnotationColor);
	const float OneOver255 = 1.0f / 255.0f;
	FLinearColor LinearAnnotationColor = FLinearColor(
		AnnotationColor.R * OneOver255,
		AnnotationColor.G * OneOver255,
		AnnotationColor.B * OneOver255,
		1.0
	);
	AnnotationMID->SetVectorParameterValue("AnnotationColor", LinearAnnotationColor);

	USceneComponent* Parent = this->GetAttachParent();
	OwnerComponent = Cast<UStaticMeshComponent>(Parent);

	if (OwnerComponent)
	{
		FPrimitiveSceneProxy* PrimitiveSceneProxy = OwnerComponent->CreateSceneProxy();
		FStaticMeshSceneProxy* StaticMeshSceneProxy = (FStaticMeshSceneProxy*)PrimitiveSceneProxy;

		UStaticMesh* StaticMesh = OwnerComponent->GetStaticMesh();
		if(StaticMesh == NULL
			|| StaticMesh->RenderData == NULL
			|| StaticMesh->RenderData->LODResources.Num() == 0
			|| StaticMesh->RenderData->LODResources[0].VertexBuffer.GetNumVertices() == 0)
		{
			return NULL;
		}

		// FPrimitiveSceneProxy* Proxy = ::new FStaticMeshSceneProxy(OwnerComponent, false);
		FPrimitiveSceneProxy* Proxy = ::new FAnnotationSceneProxy(OwnerComponent, false, AnnotationMID);
		return Proxy;
		// This is not recommended, but I know what I am doing.
	}
	else
	{
		return nullptr;
	}
}

FBoxSphereBounds UAnnotationComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	USceneComponent* Parent = this->GetAttachParent();
	UStaticMeshComponent* OwnerComponent = Cast<UStaticMeshComponent>(Parent);
	FBoxSphereBounds Bounds;
	if (OwnerComponent)
	{
		return OwnerComponent->CalcBounds(LocalToWorld);
	}
	else
	{
		return Bounds;
	}
}
