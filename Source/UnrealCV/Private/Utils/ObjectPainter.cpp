// Weichao Qiu @ 2018
#include "ObjectPainter.h"
#include "UnrealCVPrivate.h"
#include "StaticMeshResources.h"
#include "UE4CVServer.h"
#include "SceneViewport.h"
#include "Version.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"

FColor GetColorFromColorMap(int32 ObjectIndex);

FObjectPainter& FObjectPainter::Get()
{
	static FObjectPainter Singleton;
	return Singleton;
}

FExecStatus FObjectPainter::SetActorColor(FString ActorId, FColor Color)
{
	if (Id2Actor.Contains(ActorId))
	{
		AActor* Actor = Id2Actor[ActorId];
		if (PaintObject(Actor, Color))
		{
			Id2Color.Emplace(ActorId, Color);
			return FExecStatus::OK();
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Failed to paint object %s"), *ActorId));
		}
	}
	else
	{
		return FExecStatus::Error(FString::Printf(TEXT("Object %s not exist"), *ActorId));
	}
}

FExecStatus FObjectPainter::GetActorColor(FString ActorId)
{
	// Make sure the object color map is initialized
	if (Id2Color.Num() == 0)
	{
		return FExecStatus::Error(TEXT("The object list is empty, probably not initialized correctly"));
	}
	if (Id2Color.Contains(ActorId))
	{
		FColor ObjectColor = Id2Color[ActorId]; // Make sure the object exist
		FString Message = ObjectColor.ToString();
		// FString Message = "%.3f %.3f %.3f %.3f";
		return FExecStatus::OK(Message);
	}
	else
	{
		return FExecStatus::Error(FString::Printf(TEXT("Object %s not exist"), *ActorId));
	}
}

// TODO: This should be moved to command handler
FExecStatus FObjectPainter::GetObjectList()
{
	TArray<FString> Keys;
	this->Id2Actor.GetKeys(Keys);
	FString Message = "";
	for (auto ActorId : Keys)
	{
		Message += ActorId + " ";
	}
	Message = Message.LeftChop(1);
	return FExecStatus::OK(Message);
}

AActor* FObjectPainter::GetObject(FString ActorId)
{
	/** Return the pointer of an object, return NULL if object not found */
	if (Id2Actor.Contains(ActorId))
	{
		return Id2Actor[ActorId];
	}
	else
	{
		return NULL;
	}
}

/** Check whether an actor can be painted with vertex color */
bool IsPaintable(AActor* Actor)
{
	TArray<UMeshComponent*> PaintableComponents;
	Actor->GetComponents<UMeshComponent>(PaintableComponents);
	if (PaintableComponents.Num() == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void FObjectPainter::Reset(ULevel* InLevel)
{
	this->Level = InLevel;
	this->Id2Color.Empty();
	this->Id2Actor.Empty();

	// This list needs to be generated everytime the game restarted.
	check(Level);

	uint32 ObjectIndex = 0;
	for (AActor* Actor : Level->Actors)
	{
		if (Actor && IsPaintable(Actor))
		{
			FString ActorId = Actor->GetHumanReadableName();
			Id2Actor.Emplace(ActorId, Actor);
			FColor NewColor = GetColorFromColorMap(ObjectIndex);
			Id2Color.Emplace(ActorId, NewColor);
			ObjectIndex++;
		}
	}

	for (auto& Elem : Id2Color)
	{
		FString ActorId = Elem.Key;
		FColor NewColor = Elem.Value;
		AActor* Actor = Id2Actor[ActorId];
		check(PaintObject(Actor, NewColor));
	}
}


/** DisplayColor is the color that the screen will show
If DisplayColor.R = 128, the display will show 0.5 voltage
To achieve this, UnrealEngine will do gamma correction.
The value on image will be 187.
https://en.wikipedia.org/wiki/Gamma_correction#Methods_to_perform_display_gamma_correction_in_computing
*/
bool FObjectPainter::PaintObject(AActor* Actor, const FColor& Color, bool IsColorGammaEncoded)
{
	if (!Actor) return false;

	FColor NewColor;
	if (IsColorGammaEncoded)
	{
		FLinearColor LinearColor = FLinearColor::FromPow22Color(Color);
		NewColor = LinearColor.ToFColor(false);
	}
	else
	{
		NewColor = Color;
	}

	TArray<UMeshComponent*> PaintableComponents;
	Actor->GetComponents<UMeshComponent>(PaintableComponents);


	for (auto MeshComponent : PaintableComponents)
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			UStaticMesh* StaticMesh;
#if ENGINE_MINOR_VERSION >= 14  // Assume major version is 4
			StaticMesh = StaticMeshComponent->GetStaticMesh(); // This is a new function introduced in 4.14
#else
			StaticMesh = StaticMeshComponent->StaticMesh; // This is deprecated in 4.14, add here for backward compatibility
#endif
			if (StaticMesh)
			{
				uint32 NumLODLevel = StaticMesh->RenderData->LODResources.Num();
				for (uint32 PaintingMeshLODIndex = 0; PaintingMeshLODIndex < NumLODLevel; PaintingMeshLODIndex++)
				{
					FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[PaintingMeshLODIndex];
					FStaticMeshComponentLODInfo* InstanceMeshLODInfo = NULL;

					// PaintingMeshLODIndex + 1 is the minimum requirement, enlarge if not satisfied
					StaticMeshComponent->SetLODDataCount(PaintingMeshLODIndex + 1, StaticMeshComponent->LODData.Num());
					InstanceMeshLODInfo = &StaticMeshComponent->LODData[PaintingMeshLODIndex];

					InstanceMeshLODInfo->ReleaseOverrideVertexColorsAndBlock();
					// Setup OverrideVertexColors
					// if (!InstanceMeshLODInfo->OverrideVertexColors) // TODO: Check this
					{
						InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer;

						FColor FillColor = FColor(255, 255, 255, 255);
						InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(FColor::White, LODModel.GetNumVertices());
					}

					uint32 NumVertices = LODModel.GetNumVertices();
					check(InstanceMeshLODInfo->OverrideVertexColors);
					check(NumVertices <= InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices());
					// StaticMeshComponent->CachePaintedDataIfNecessary();

					for (uint32 ColorIndex = 0; ColorIndex < NumVertices; ++ColorIndex)
					{
						// LODModel.ColorVertexBuffer.VertexColor(ColorIndex) = NewColor;  // This is vertex level
						// Need to initialize the vertex buffer first
						uint32 NumOverrideVertexColors = InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices();
						uint32 NumPaintedVertices = InstanceMeshLODInfo->PaintedVertices.Num();
						// check(NumOverrideVertexColors == NumPaintedVertices);
						InstanceMeshLODInfo->OverrideVertexColors->VertexColor(ColorIndex) = NewColor;
						// InstanceMeshLODInfo->PaintedVertices[ColorIndex].Color = NewColor;
					}
					BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);
					StaticMeshComponent->MarkRenderStateDirty();
					// BeginUpdateResourceRHI(InstanceMeshLODInfo->OverrideVertexColors);


					/*
					// TODO: Need to check other LOD levels
					// Use flood fill to paint mesh vertices
					UE_LOG(LogUnrealCV, Warning, TEXT("%s:%s has %d vertices"), *Actor->GetActorLabel(), *StaticMeshComponent->GetName(), NumVertices);

					if (LODModel.ColorVertexBuffer.GetNumVertices() == 0)
					{
					// Mesh doesn't have a color vertex buffer yet!  We'll create one now.
					LODModel.ColorVertexBuffer.InitFromSingleColor(FColor(255, 255, 255, 255), LODModel.GetNumVertices());
					}

					*/
				}
			}
		}
	}
	return true;
}
