// Weichao Qiu @ 2019
#include "CubeActor.h"

#include "UObject/ConstructorHelpers.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"

ACubeActor::ACubeActor() : Super()
{
	FString CubeMeshPath = "StaticMesh'/Engine/BasicShapes/Cube.Cube'";
	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshObjFinder(*CubeMeshPath);
	this->GetStaticMeshComponent()->SetStaticMesh(MeshObjFinder.Object);
}
