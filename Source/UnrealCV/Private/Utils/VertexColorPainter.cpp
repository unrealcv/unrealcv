// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "VertexColorPainter.h"

void PaintStaticMesh(UStaticMeshComponent* StaticMeshComponent, const FColor& VertexColor);
void PaintSkelMesh(USkinnedMeshComponent* SkinnedMeshComponent, const FColor& VertexColor);

void FVertexColorPainter::PaintVertexColor(AActor* Actor, const FColor& PaintColor)
{
	TArray<UMeshComponent*> PaintableComponents;
	Actor->GetComponents<UMeshComponent>(PaintableComponents);

	for (auto MeshComponent : PaintableComponents)
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			PaintStaticMesh(StaticMeshComponent, PaintColor);
		}
		if (USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(MeshComponent))
		{
			PaintSkelMesh(SkinnedMeshComponent, PaintColor);
		}
	}
}

void PaintStaticMesh(UStaticMeshComponent* StaticMeshComponent, const FColor& VertexColor)
{
	UStaticMesh* StaticMesh;
#if ENGINE_MINOR_VERSION >= 14  // Assume major version is 4
	StaticMesh = StaticMeshComponent->GetStaticMesh(); // This is a new function introduced in 4.14
#else
	StaticMesh = StaticMeshComponent->StaticMesh; // This is deprecated in 4.14, add here for backward compatibility
#endif
	if (!StaticMesh)
	{
		return;
	}

	uint32 NumLODLevel = StaticMesh->RenderData->LODResources.Num();
	for (uint32 PaintingMeshLODIndex = 0; PaintingMeshLODIndex < NumLODLevel; PaintingMeshLODIndex++)
	{
		FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[PaintingMeshLODIndex];
		FStaticMeshComponentLODInfo* InstanceMeshLODInfo = NULL;

		// PaintingMeshLODIndex + 1 is the minimum requirement, enlarge if not satisfied
		StaticMeshComponent->SetLODDataCount(PaintingMeshLODIndex + 1, StaticMeshComponent->LODData.Num());
		InstanceMeshLODInfo = &StaticMeshComponent->LODData[PaintingMeshLODIndex];

		InstanceMeshLODInfo->ReleaseOverrideVertexColorsAndBlock();
		InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer;
		check(InstanceMeshLODInfo->OverrideVertexColors);

		InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(VertexColor, LODModel.GetNumVertices());

		BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);
		StaticMeshComponent->MarkRenderStateDirty();
	}
}

void PaintSkelMesh(USkinnedMeshComponent* SkinnedMeshComponent, const FColor& VertexColor)
{
	USkeletalMesh* SkeletalMesh = SkinnedMeshComponent->SkeletalMesh;
	if (!SkeletalMesh)
	{
		return;
	}

	TIndirectArray<FStaticLODModel>& LODModels = SkeletalMesh->GetResourceForRendering()->LODModels;
	uint32 NumLODLevel = LODModels.Num();
	// SkinnedMeshComponent->SetLODDataCount(NumLODLevel, NumLODLevel);

	for (uint32 LODIndex = 0; LODIndex < NumLODLevel; LODIndex++)
	{
		FStaticLODModel& LODModel = LODModels[LODIndex];
		FSkelMeshComponentLODInfo* LODInfo = &SkinnedMeshComponent->LODInfo[LODIndex];
		if (LODInfo->OverrideVertexColors)
		{
			BeginReleaseResource(LODInfo->OverrideVertexColors);
			// Ensure the RT no longer accessed the data, might slow down
			FlushRenderingCommands();
			// The RT thread has no access to it any more so it's safe to delete it.
			// CleanUp();
		}
		// LODInfo->ReleaseOverrideVertexColorsAndBlock();
		LODInfo->OverrideVertexColors = new FColorVertexBuffer;
		LODInfo->OverrideVertexColors->InitFromSingleColor(VertexColor, LODModel.NumVertices);

		BeginInitResource(LODInfo->OverrideVertexColors);
	}
	SkinnedMeshComponent->MarkRenderStateDirty();
}
