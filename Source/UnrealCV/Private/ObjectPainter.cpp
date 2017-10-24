#include "UnrealCVPrivate.h"
#include "ObjectPainter.h"
#include "StaticMeshResources.h"
#include "UE4CVServer.h"
#include "SceneViewport.h"
#include "Version.h"
#include "ColorMap.h"

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

void FObjectPainter::Reset(ULevel* InLevel)
{
	this->Level = InLevel;
	this->Id2Color.Empty();
	this->Id2Actor.Empty();

	// This list needs to be generated everytime the game restarted.
	check(Level);

	uint32 ObjectIndex = 0;
	// for (AActor* Actor : Level->Actors)
	for (TActorIterator<AActor> ActorItr(FUE4CVServer::Get().GetGameWorld()); ActorItr; ++ActorItr)
	{
		AActor *Actor = *ActorItr;
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
			PaintStaticMesh(StaticMeshComponent, NewColor);
		}
		if (USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(MeshComponent))
		{
			PaintSkelMesh(SkinnedMeshComponent, NewColor);
		}
		
	}
	return true;
}