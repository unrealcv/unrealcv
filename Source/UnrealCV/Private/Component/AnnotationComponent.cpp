#include "AnnotationComponent.h"

#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Runtime/Engine/Public/MaterialShared.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

#include "SkeletalMeshRenderData.h"
#include "UnrealcvLog.h"
// For UE4 < 19
// check https://github.com/unrealcv/unrealcv/blob/1369a72be8428547318d8a52ae2d63e1eb57a001/Source/UnrealCV/Private/Component/AnnotationComponent.cpp#L11

/** Store mesh data of a parent mesh component */
class FParentMeshInfo
{
public:

	FParentMeshInfo(USceneComponent* ParentComponent)
	{
		ParentMeshType = EParentMeshType::None;

		if (!IsValid(ParentComponent))
		{ 
			UE_LOG(LogTemp, Warning, TEXT("ParentComponent is invalid."));
			return;
		}

		UStaticMeshComponent* InStaticMeshComponent = Cast<UStaticMeshComponent>(ParentComponent);
		USkeletalMeshComponent* InSkelMeshComponent = Cast<USkeletalMeshComponent>(ParentComponent);
		if (IsValid(InStaticMeshComponent))
		{
			StaticMeshComponent = InStaticMeshComponent;
			ParentMeshType = EParentMeshType::StaticMesh;
		}
		else if (IsValid(InSkelMeshComponent))
		{
			SkelMeshComponent = InSkelMeshComponent;
			CachedMeshObject = SkelMeshComponent->MeshObject;
			CachedSkeletalMesh = SkelMeshComponent->SkeletalMesh;
			ParentMeshType = EParentMeshType::SkelMesh;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("The ParentComponent is of an unrecognized type : %s."), *ParentComponent->GetClass()->GetName());
		}
	}

	bool RequiresUpdate()
	{
		switch (ParentMeshType)
		{
		case EParentMeshType::StaticMesh:
			return false;
		case EParentMeshType::SkelMesh:
			if (CachedMeshObject != SkelMeshComponent->MeshObject
			|| CachedSkeletalMesh != SkelMeshComponent->SkeletalMesh)
			{
				return true;
			}
			else
			{
				return false;
			}
			break;
		case EParentMeshType::None: // The ParentMeshType is invalid
			return true;
		default: // The ParentMeshType is invalid
			return true;
		}
	}

	UMeshComponent* GetParentMeshComponent()
	{
		switch (ParentMeshType)
		{
		case EParentMeshType::None:
			return nullptr;
		case EParentMeshType::StaticMesh:
			return StaticMeshComponent.Get();
		case EParentMeshType::SkelMesh:
			return SkelMeshComponent.Get();
		default:
			return nullptr;
		}

	}

private:
	FSkeletalMeshObject* CachedMeshObject;
	USkeletalMesh* CachedSkeletalMesh;
	TWeakObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	TWeakObjectPtr<USkeletalMeshComponent> SkelMeshComponent;
	EParentMeshType ParentMeshType;
};

// Inheritance is needed because I need to access protected data
// Note: in the show Material command, some area might be not colored, this is caused by the issue that
// both the original mesh and the annotation mesh are rendered, this is not an issue for the AnnotationSensor, which will exclude original meshes.
class FStaticAnnotationSceneProxy : public FStaticMeshSceneProxy
{
public:
	FMaterialRenderProxy* MaterialRenderProxy;

	FStaticAnnotationSceneProxy(UStaticMeshComponent* Component, bool bForceLODsShareStaticLighting, UMaterialInterface* AnnotationMID) :
		FStaticMeshSceneProxy(Component, bForceLODsShareStaticLighting)
	{
		// MaterialRenderProxy = new FColoredMaterialRenderProxy(GEngine->DebugMeshMaterial->GetRenderProxy(false), FLinearColor::Red);
		MaterialRenderProxy = AnnotationMID->GetRenderProxy(false, false);
		// FIXME: release this later.

		this->MaterialRelevance = AnnotationMID->GetRelevance(GetScene().GetFeatureLevel());
		// Note: This MaterailRelevance makes no difference?

		this->bVerifyUsedMaterials = false;
		// From StaticMeshRenderer

		/* Not needed anymore
		int32 NumLODs = RenderData->LODResources.Num();
		// for(int32 LODIndex = ClampedMinLOD; LODIndex < NumLODs; LODIndex++)
		for(int32 LODIndex = 0; LODIndex < NumLODs; LODIndex++)
		{
			const FStaticMeshLODResources& LODModel = RenderData->LODResources[LODIndex];
			FLODInfo& ProxyLODInfo = LODs[LODIndex];
			for (int32 SectionIndex = 0; SectionIndex < LODModel.Sections.Num(); SectionIndex++)
			{
				ProxyLODInfo.Sections[SectionIndex].Material = AnnotationMID;
				// const FMaterial* Material = ProxyLODInfo.Sections[SectionIndex].Material->GetRenderProxy(false)->GetMaterial(FeatureLevel);
				// ProxyLODInfo.Sections[SectionIndex].Material = GEngine->VertexColorViewModeMaterial_ColorOnly;
				// ProxyLODInfo.Sections[SectionIndex].Material = UMaterial::GetDefaultMaterial(MD_Surface);
				// Note: why sometimes this AnnotationMID looks like fallback default material?
			}
		}
		*/
		bCastShadow = false;
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
	if (View->Family->EngineShowFlags.Materials)
	{
		FPrimitiveViewRelevance ViewRelevance;
		ViewRelevance.bDrawRelevance = 0; // This will make it get ignored
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
	bool Ret = FStaticMeshSceneProxy::GetMeshElement(LODIndex, BatchIndex, ElementIndex, InDepthPriorityGroup,
		bUseSelectedMaterial, bUseHoveredMaterial, bAllowPreCulledIndices, OutMeshBatch);
	OutMeshBatch.MaterialRenderProxy = this->MaterialRenderProxy;
	return Ret;
}

class FSkeletalAnnotationSceneProxy : public FSkeletalMeshSceneProxy
{
public:
	FSkeletalAnnotationSceneProxy(const USkinnedMeshComponent* Component, FSkeletalMeshRenderData* InSkeletalMeshRenderData, UMaterialInterface* AnnotationMID)
	: FSkeletalMeshSceneProxy(Component, InSkeletalMeshRenderData)
	{
		// TODO: Update MaterialRelevance
		this->bVerifyUsedMaterials = false;
		// this->bCastShadow = false;
		this->bCastDynamicShadow = false;
		for(int32 LODIdx=0; LODIdx < LODSections.Num(); LODIdx++)
		{
			FLODSectionElements& LODSection = LODSections[LODIdx];
			for(int32 SectionIndex = 0; SectionIndex < LODSection.SectionElements.Num(); SectionIndex++)
			{
				if (IsValid(AnnotationMID))
				{
					LODSection.SectionElements[SectionIndex].Material = AnnotationMID;
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

FPrimitiveViewRelevance FSkeletalAnnotationSceneProxy::GetViewRelevance(const FSceneView * View) const
{
	if (View->Family->EngineShowFlags.Materials)
	{
		FPrimitiveViewRelevance ViewRelevance;
		ViewRelevance.bDrawRelevance = 0; // This will make it gets ignored, when materials flag is enabled.
		return ViewRelevance;
	}
	else
	{
		return FSkeletalMeshSceneProxy::GetViewRelevance(View);
	}
}

UAnnotationComponent::UAnnotationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), ParentMeshInfo(nullptr)
{
	FString MaterialPath = TEXT("Material'/UnrealCV/AnnotationColor.AnnotationColor'");
	// FString MeterialPath = TEXT("MaterialInstanceConstant'/UnrealCV/AnnotationColor_Inst.AnnotationColor_Inst'");
	// static ConstructorHelpers::FObjectFinder<UMaterialInstanceDynamic> AnnotationMaterialObject(*MaterialPath);
	static ConstructorHelpers::FObjectFinder<UMaterial> AnnotationMaterialObject(*MaterialPath);
	// UMaterialInstanceDynamic* AnnotationMaterial = AnnotationMaterialObject.Object;
	AnnotationMaterial = AnnotationMaterialObject.Object;
	ParentMeshInfo = MakeShareable(new FParentMeshInfo(nullptr)); // This will be invalid until attached to a MeshComponent

	this->PrimaryComponentTick.bCanEverTick = true;
}

void UAnnotationComponent::OnRegister()
{
	Super::OnRegister();

	// Note: This can not be placed in the constructor, MID means material instance dynamic
	AnnotationMID = UMaterialInstanceDynamic::Create(AnnotationMaterial, this, TEXT("AnnotationMaterialMID"));
	if (!IsValid(AnnotationMID))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("AnnotationMaterial is not correctly initialized"));
		return;
	}

	SetAnnotationColor(this->AnnotationColor);
	ParentMeshInfo = MakeShareable(new FParentMeshInfo(this->GetAttachParent()));
}

void UAnnotationComponent::SetAnnotationColor(FColor NewAnnotationColor)
{
	this->AnnotationColor = NewAnnotationColor;
	const float OneOver255 = 1.0f / 255.0f; // TODO: Check 255 or 256?
	FLinearColor LinearAnnotationColor = FLinearColor(
		AnnotationColor.R * OneOver255,
		AnnotationColor.G * OneOver255,
		AnnotationColor.B * OneOver255,
		1.0
	);

	if (IsValid(AnnotationMID))
	{
		// FLinearColor LinearAnnotationColor = FLinearColor::MakeRandomColor();
		// FLinearColor LinearAnnotationColor = FLinearColor::White;
		AnnotationMID->SetVectorParameterValue("AnnotationColor", LinearAnnotationColor);
		// Note: The "exposure compensation" in "PostProcessVolume3" in the RR map will destroy the color
		// Note: Saturate the color to 1. This is a mysterious behavior after tedious debug.
	}
}

FColor UAnnotationComponent::GetAnnotationColor()
{
	return AnnotationColor;
}

// TODO: This needs to be involked when the ParentComponent refresh its render state, otherwise it will crash the engine
FPrimitiveSceneProxy* UAnnotationComponent::CreateSceneProxy()
{
	// UMaterialInstanceDynamic* AnnotationMID = UMaterialInstanceDynamic::Create(AnnotationMaterial, this);
	// FColor AnnotationColor = FColor::MakeRandomColor();
	// AnnotationMID->SetVectorParameterByIndex(0, AnnotationColor);


	// USceneComponent* Parent = this->GetAttachParent();
	USceneComponent* ParentComponent = this->ParentMeshInfo->GetParentMeshComponent();

	if (!IsValid(ParentComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Parent component is invalid."));
		return nullptr;
	}

	UMaterialInterface* ProxyMaterial = AnnotationMID; // Material Instance Dynamic
	// UMaterialInterface* ProxyMaterial = AnnotationMaterial;

	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(ParentComponent);
	USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ParentComponent);
	if (IsValid(StaticMeshComponent))
	{
		// FPrimitiveSceneProxy* PrimitiveSceneProxy = StaticMeshComponent->CreateSceneProxy();
		// FStaticMeshSceneProxy* StaticMeshSceneProxy = (FStaticMeshSceneProxy*)PrimitiveSceneProxy;

		UStaticMesh* ParentStaticMesh = StaticMeshComponent->GetStaticMesh();
		if(ParentStaticMesh == NULL
			|| ParentStaticMesh->RenderData == NULL
			|| ParentStaticMesh->RenderData->LODResources.Num() == 0)
			// || StaticMesh->RenderData->LODResources[0].VertexBuffer.GetNumVertices() == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("ParentStaticMesh is invalid."));
			return NULL;
		}

		// FPrimitiveSceneProxy* Proxy = ::new FStaticMeshSceneProxy(OwnerComponent, false);
		FPrimitiveSceneProxy* Proxy = ::new FStaticAnnotationSceneProxy(StaticMeshComponent, false, ProxyMaterial);
		return Proxy;
		// This is not recommended, but I know what I am doing.
	}
	else if (IsValid(SkeletalMeshComponent))
	{
		ERHIFeatureLevel::Type SceneFeatureLevel = GetWorld()->FeatureLevel;

		// Ref: https://github.com/EpicGames/UnrealEngine/blob/4.19/Engine/Source/Runtime/Engine/Private/Components/SkinnedMeshComponent.cpp#L415
		FSkeletalMeshRenderData* SkelMeshRenderData = SkeletalMeshComponent->GetSkeletalMeshRenderData();

		// Only create a scene proxy for rendering if properly initialized
		if (SkelMeshRenderData &&
			SkelMeshRenderData->LODRenderData.IsValidIndex(SkeletalMeshComponent->PredictedLODLevel) &&
			SkeletalMeshComponent->MeshObject) // The risk of using MeshObject
		{
			// Only create a scene proxy if the bone count being used is supported, or if we don't have a skeleton (this is the case with destructibles)
			// int32 MaxBonesPerChunk = SkelMeshResource->GetMaxBonesPerSection();
			// if (MaxBonesPerChunk <= GetFeatureLevelMaxNumberOfBones(SceneFeatureLevel))
			// {
			//	Result = ::new FSkeletalAnnotationSceneProxy(SkeletalMeshComponent, SkelMeshResource, AnnotationMID);
			// }
			// TODO: The SkeletalMeshComponent might need to be recreated
			return new FSkeletalAnnotationSceneProxy(SkeletalMeshComponent, SkelMeshRenderData, ProxyMaterial);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("The data of SkeletalMeshComponent is invalid."), *ParentComponent->GetClass()->GetName());
			return nullptr;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("The type of ParentMeshComponent : %s can not be supported."), *ParentComponent->GetClass()->GetName());
		return nullptr;
	}
	return nullptr;
}

FBoxSphereBounds UAnnotationComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	// UMeshComponent* ParentMeshComponent = ParentMeshInfo->GetParentMeshComponent();
	// if (IsValid(ParentMeshComponent))
	// {
	// 	return ParentMeshComponent->CalcBounds(LocalToWorld);
	// }
	// else
	// {
	// 	FBoxSphereBounds DefaultBounds;
	// 	return DefaultBounds;
	// }

	USceneComponent* Parent = this->GetAttachParent();
	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Parent);
	if (IsValid(StaticMeshComponent))
	{
		return StaticMeshComponent->CalcBounds(LocalToWorld);
	}

	USkeletalMeshComponent* SkeletalMeshComponent = Cast<USkeletalMeshComponent>(Parent);
	if (IsValid(SkeletalMeshComponent))
	{
		return SkeletalMeshComponent->CalcBounds(LocalToWorld);
	}

	FBoxSphereBounds DefaultBounds;
	return DefaultBounds;
}

// Extra overhead for the game scene
void UAnnotationComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction); 

	// if (ParentMeshInfo->RequiresUpdate()) 
	// TODO: This sometimes miss a required update, see OWIMap. Not sure why.
	// TODO: Per-frame update is certainly wasted.
	{
		// FIXME: Update the render proxy per frame will cause jittering on the material.
		ParentMeshInfo = MakeShareable(new FParentMeshInfo(this->GetAttachParent()));
		MarkRenderStateDirty();
	}
}
