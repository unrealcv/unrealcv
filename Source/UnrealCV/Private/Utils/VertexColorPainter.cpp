// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "VertexColorPainter.h"

void FVertexColorPainter::PaintVertexColor(AActor* Actor, const FColor& PaintColor)
{
	TArray<UMeshComponent*> PaintableComponents;
	Actor->GetComponents<UMeshComponent>(PaintableComponents);

	for (auto MeshComponent : PaintableComponents)
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			UStaticMesh* StaticMesh;
			StaticMesh = StaticMeshComponent->GetStaticMesh(); // This is a new function introduced in 4.14

			if (StaticMesh)
			{
				uint32 NumLODLevel = StaticMesh->RenderData->LODResources.Num();
				for (uint32 PaintingMeshLODIndex = 0; PaintingMeshLODIndex < NumLODLevel; PaintingMeshLODIndex++)
				{
					FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[PaintingMeshLODIndex];
					FStaticMeshComponentLODInfo* InstanceMeshLODInfo = NULL;

					StaticMeshComponent->SetLODDataCount(PaintingMeshLODIndex + 1, StaticMeshComponent->LODData.Num());
					InstanceMeshLODInfo = &StaticMeshComponent->LODData[PaintingMeshLODIndex];
					InstanceMeshLODInfo->ReleaseOverrideVertexColorsAndBlock();

					InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer;
					InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(PaintColor, LODModel.GetNumVertices());

					BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);
					StaticMeshComponent->MarkRenderStateDirty();
				}
			}
		}
	}
}
