#include "UnrealCVPrivate.h"
#include "AnnotationComponent.h"
#include "SkeletalMeshTypes.h"
#if ENGINE_MINOR_VERSION >= 19
#include "SkeletalMeshRenderData.h"
#endif

// Inheritance is needed because I need to access protected data
class FStaticAnnotationSceneProxy : public FStaticMeshSceneProxy
{
	/*static void OverrideMaterial(FStaticMeshSceneProxy* StaticMeshSceneProxy)
	{
		StaticMeshSceneProxy->LODs;
	}*/

public:
	FStaticAnnotationSceneProxy(UStaticMeshComponent* Component, bool bForceLODsShareStaticLighting, UMaterialInterface* AnnotationMaterial) :
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

FPrimitiveViewRelevance FStaticAnnotationSceneProxy::GetViewRelevance(const FSceneView * View) const
{
	// View->Family->EngineShowFlags.
	FPrimitiveViewRelevance ViewRelevance;
	ViewRelevance.bDrawRelevance = 0; // This will make it get ignored
	if (View->Family->EngineShowFlags.Materials)
	{
		return ViewRelevance;
	}
	else
	{
		return FStaticMeshSceneProxy::GetViewRelevance(View);
	}
}


void FStaticAnnotationSceneProxy::GetDynamicMeshElements(
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

bool FStaticAnnotationSceneProxy::GetMeshElement(
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

#if ENGINE_MINOR_VERSION >= 19
class FSkeletalAnnotationSceneProxy : public FSkeletalMeshSceneProxy
{
public:
	FSkeletalAnnotationSceneProxy(const USkinnedMeshComponent* Component, FSkeletalMeshRenderData* InSkeletalMeshRenderData, UMaterialInterface* AnnotationMaterial)
	: FSkeletalMeshSceneProxy(Component, InSkeletalMeshRenderData)
	{
		this->bVerifyUsedMaterials = false;
		// this->bCastShadow = false;
		this->bCastDynamicShadow = false;
		for(int32 LODIdx=0; LODIdx < LODSections.Num(); LODIdx++)
		{
			FLODSectionElements& LODSection = LODSections[LODIdx];
			for(int32 SectionIndex = 0; SectionIndex < LODSection.SectionElements.Num(); SectionIndex++)
			{
				if (IsValid(AnnotationMaterial))
				{
					LODSection.SectionElements[SectionIndex].Material = AnnotationMaterial;
				}
				else
				{
					UE_LOG(LogUnrealCV, Warning, TEXT("AnnotationMaterial is Invalid in FSkeletalSceneProxy"));
				}
			}
		}
	}
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;
};
#endif

#if ENGINE_MINOR_VERSION < 19
class FSkeletalAnnotationSceneProxy : public FSkeletalMeshSceneProxy
{
public:
	FSkeletalAnnotationSceneProxy(const USkinnedMeshComponent* Component, FSkeletalMeshResource* InSkelMeshResource, UMaterialInterface* AnnotationMaterial)
	: FSkeletalMeshSceneProxy(Component, InSkelMeshResource)
	{
		this->bVerifyUsedMaterials = false;
		// this->bCastShadow = false;
		this->bCastDynamicShadow = false;
		for(int32 LODIdx=0; LODIdx < SkelMeshResource->LODModels.Num(); LODIdx++)
		{
			const FStaticLODModel& LODModel = SkelMeshResource->LODModels[LODIdx];
			// const FSkeletalMeshLODInfo& Info = Component->SkeletalMesh->LODInfo[LODIdx];
			FLODSectionElements& LODSection = LODSections[LODIdx];
			for(int32 SectionIndex = 0;SectionIndex < LODModel.Sections.Num();SectionIndex++)
			{
				if (IsValid(AnnotationMaterial))
				{
					LODSection.SectionElements[SectionIndex].Material = AnnotationMaterial;
				}
				else
				{
					UE_LOG(LogUnrealCV, Warning, TEXT("AnnotationMaterial is Invalid in FSkeletalSceneProxy"));
				}
			}
		}
	}
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView * View) const override;
};
#endif

FPrimitiveViewRelevance FSkeletalAnnotationSceneProxy::GetViewRelevance(const FSceneView * View) const
{
	// View->Family->EngineShowFlags.
	FPrimitiveViewRelevance ViewRelevance;
	ViewRelevance.bDrawRelevance = 0; // This will make it get ignored
	if (View->Family->EngineShowFlags.Materials)
	{
		return ViewRelevance;
	}
	else
	{
		return FSkeletalMeshSceneProxy::GetViewRelevance(View);
	}
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
	StaticMeshComponent = Cast<UStaticMeshComponent>(Parent);
	SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Parent);

	if (IsValid(StaticMeshComponent))
	{
		// FPrimitiveSceneProxy* PrimitiveSceneProxy = StaticMeshComponent->CreateSceneProxy();
		// FStaticMeshSceneProxy* StaticMeshSceneProxy = (FStaticMeshSceneProxy*)PrimitiveSceneProxy;

		UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
		if(StaticMesh == NULL
			|| StaticMesh->RenderData == NULL
			|| StaticMesh->RenderData->LODResources.Num() == 0)
			// || StaticMesh->RenderData->LODResources[0].VertexBuffer.GetNumVertices() == 0)
		{
			return NULL;
		}

		if (!IsValid(AnnotationMID))
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("AnnotationMaterial is not correctly initialized in CreateSceneProxy for StaticMesh"));
		}
		// FPrimitiveSceneProxy* Proxy = ::new FStaticMeshSceneProxy(OwnerComponent, false);
		FPrimitiveSceneProxy* Proxy = ::new FStaticAnnotationSceneProxy(StaticMeshComponent, false, AnnotationMID);
		return Proxy;
		// This is not recommended, but I know what I am doing.
	}

	if (IsValid(SkeletalMeshComponent))
	{
		ERHIFeatureLevel::Type SceneFeatureLevel = GetWorld()->FeatureLevel;

#if ENGINE_MINOR_VERSION < 19
		FSkeletalMeshResource* SkelMeshResource = SkeletalMeshComponent->GetSkeletalMeshResource();

		// Only create a scene proxy for rendering if properly initialized
		if (SkelMeshResource &&
			SkelMeshResource->LODModels.IsValidIndex(SkeletalMeshComponent->PredictedLODLevel) &&
			!SkeletalMeshComponent->bHideSkin &&
			SkeletalMeshComponent->MeshObject)
		{
			// Only create a scene proxy if the bone count being used is supported, or if we don't have a skeleton (this is the case with destructibles)
			// int32 MaxBonesPerChunk = SkelMeshResource->GetMaxBonesPerSection();
			// if (MaxBonesPerChunk <= GetFeatureLevelMaxNumberOfBones(SceneFeatureLevel))
			// {
			//	Result = ::new FSkeletalAnnotationSceneProxy(SkeletalMeshComponent, SkelMeshResource, AnnotationMID);
			// }
			if (!IsValid(AnnotationMID))
			{
				UE_LOG(LogUnrealCV, Warning, TEXT("AnnotationMaterial is not correctly initialized in CreateSceneProxy for SkeletalMesh"));
			}
			return new FSkeletalAnnotationSceneProxy(SkeletalMeshComponent, SkelMeshResource, AnnotationMID);
		}
#else
		// Ref: https://github.com/EpicGames/UnrealEngine/blob/4.19/Engine/Source/Runtime/Engine/Private/Components/SkinnedMeshComponent.cpp#L415
		FSkeletalMeshRenderData* SkelMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();

		// Only create a scene proxy for rendering if properly initialized
		if (SkelMeshRenderData &&
			SkelMeshRenderData->LODRenderData.IsValidIndex(SkeletalMeshComponent->PredictedLODLevel) &&
			SkeletalMeshComponent->MeshObject)
		{
			// Only create a scene proxy if the bone count being used is supported, or if we don't have a skeleton (this is the case with destructibles)
			// int32 MaxBonesPerChunk = SkelMeshResource->GetMaxBonesPerSection();
			// if (MaxBonesPerChunk <= GetFeatureLevelMaxNumberOfBones(SceneFeatureLevel))
			// {
			//	Result = ::new FSkeletalAnnotationSceneProxy(SkeletalMeshComponent, SkelMeshResource, AnnotationMID);
			// }
			if (!IsValid(AnnotationMID))
			{
				UE_LOG(LogUnrealCV, Warning, TEXT("AnnotationMaterial is not correctly initialized in CreateSceneProxy for SkeletalMesh"));
			}
			return new FSkeletalAnnotationSceneProxy(SkeletalMeshComponent, SkelMeshRenderData, AnnotationMID);
		}
#endif
	}

	return nullptr;
}

FBoxSphereBounds UAnnotationComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	USceneComponent* Parent = this->GetAttachParent();
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Parent);
	USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Parent);

	FBoxSphereBounds Bounds;
	if (IsValid(StaticMeshComponent))
	{
		return StaticMeshComponent->CalcBounds(LocalToWorld);
	}

	if (IsValid(SkeletalMeshComponent))
	{
		return SkeletalMeshComponent->CalcBounds(LocalToWorld);
	}

	return Bounds;
}