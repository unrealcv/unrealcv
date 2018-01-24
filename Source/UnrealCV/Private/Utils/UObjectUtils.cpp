#include "UnrealCVPrivate.h"
#include "UObjectUtils.h"

AActor* GetActorById(UWorld* World, FString ActorId)
{
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (Actor->GetWorld() == World && Actor->GetName() == ActorId)
		{
			return Actor;
		}
	}
	return nullptr;
}

UObject* GetObjectById(UWorld* World, FString ObjectId)
{
	for (TObjectIterator<UObject> ObjItr; ObjItr; ++ObjItr)
	{
		UObject* Obj = *ObjItr;
		if (Obj->GetWorld() == World && Obj->GetName() == ObjectId)
		{
			return Obj;
		}
	}
	return nullptr;
}
