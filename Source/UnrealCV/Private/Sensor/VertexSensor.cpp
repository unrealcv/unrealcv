// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "VertexSensor.h"


// Use vget /object/[id]/vertex json?
FVertexSensor::FVertexSensor(AActor* InActor)
{
	this->OwnerActor = InActor;
}

TArray<FVector> SkinnedMeshComponentGetVertexArray(USkinnedMeshComponent* Component)
{
	TArray<FVector> VertexArray;
	if (!IsValid(Component)) return VertexArray;

	Component->ComputeSkinnedPositions(VertexArray);

	return VertexArray;
	// SkinnedMeshComponent.cpp::ComputeSkinnedPositions
	// for (int VertexIndex = 0; VertexIndex < ; VertexIndex++)
	// {
	// 	Component->GetSkinnedVertexPosition(VertexIndex);
	// }
	// USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	// if (!IsValid(SkeletalMesh)) return;
    //
	// TIndirectArray<FStaticLODModel>& LODModels = SkeletalMesh->GetResourceForRendering()->LODModels;
    //
	// uint32 NumLODLevel = LODModels.Num();
    //
	// for (uint32 LODIndex = 0; LODIndex < NumLODLevel; LODIndex++)
	// {
	// }
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
	TArray<FVector> VertexArray;
	if (!IsValid(this->OwnerActor)) return VertexArray;

	TArray<UMeshComponent*> PaintableComponents;

	this->OwnerActor->GetComponents<UMeshComponent>(PaintableComponents);

	for (auto MeshComponent : PaintableComponents)
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			TArray<FVector> ComponentVertexArray = StaticMeshComponentGetVertexArray(StaticMeshComponent);
			VertexArray.Append(ComponentVertexArray);
		}
		if (USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(MeshComponent))
		{
			TArray<FVector> ComponentVertexArray = SkinnedMeshComponentGetVertexArray(SkinnedMeshComponent);
			VertexArray.Append(ComponentVertexArray);
		}
	}
	return VertexArray;
}
