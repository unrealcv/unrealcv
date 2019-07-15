#include "UnrealCVPrivate.h"
#include "ObjectPainter.h"
#include "StaticMeshResources.h"
#include "UE4CVServer.h"
#include "Slate/SceneViewport.h"
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
