// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "VertexSensor.h"


// Use vget /object/[id]/vertex json?
FVertexSensor::FVertexSensor(AActor* InActor)
{
	this->OnwerActor = InActor;
}

TArray<FVector> StaticMeshComponentGetVertexArray(UStaticMeshComponent* StaticMeshComponent)
{
	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
	TArray<FVector> VertexArray;

	if (StaticMesh)
	{
		uint32 NumLODLevel = StaticMesh->RenderData->LODResources.Num();
		for (uint32 LODIndex = 0; LODIndex < NumLODLevel; LODIndex++)
		{
			FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[LODIndex];
			FStaticMeshComponentLODInfo* InstanceMeshLODInfo = NULL;

			uint32 NumVertices = LODModel.GetNumVertices();

			FPositionVertexBuffer& PositionVertexBuffer = LODModel.PositionVertexBuffer;
			for (uint32 VertexIndex = 0; VertexIndex < PositionVertexBuffer.GetNumVertices(); VertexIndex++)
			{
				FVector Position = PositionVertexBuffer.VertexPosition(VertexIndex);
				VertexArray.Add(Position);
			}

			//FStaticMeshVertexBuffer& VertexBuffer = LODModel.VertexBuffer;
			//const uint8* Data = VertexBuffer.GetRawVertexData();

			//FRawStaticIndexBuffer& IndexBuffer = LODModel.IndexBuffer;
			//FIndexArrayView IndexArrayView = IndexBuffer.GetArrayView();
		}
	}

	return VertexArray;
}

TArray<FVector> FVertexSensor::GetVertexArray()
{
	TArray<UMeshComponent*> PaintableComponents;
	this->OnwerActor->GetComponents<UMeshComponent>(PaintableComponents);

	TArray<FVector> VertexArray;
	for (auto MeshComponent : PaintableComponents)
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			VertexArray = StaticMeshComponentGetVertexArray(StaticMeshComponent);
			// PaintStaticMesh(StaticMeshComponent, PaintColor);
		}
		if (USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(MeshComponent))
		{
			// PaintSkelMesh(SkinnedMeshComponent, PaintColor);
		}
	}
	return VertexArray;
}
