// Weichao Qiu @ 2017
#include "VertexSensor.h"
#include "Runtime/Engine/Classes/Components/MeshComponent.h"
#include "Runtime/Engine/Classes/Components/SkinnedMeshComponent.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "VisionBPLib.h"

// Use vget /object/[id]/vertex json?
FVertexSensor::FVertexSensor(const AActor* InActor)
{
	this->OwnerActor = InActor;
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
			TArray<FVector> ComponentVertexArray = UVisionBPLib::StaticMeshComponentGetVertexArray(StaticMeshComponent);
			VertexArray.Append(ComponentVertexArray);
		}
		if (USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(MeshComponent))
		{
			TArray<FVector> ComponentVertexArray = UVisionBPLib::SkinnedMeshComponentGetVertexArray(SkinnedMeshComponent);
			VertexArray.Append(ComponentVertexArray);
		}
	}
	return VertexArray;
}
