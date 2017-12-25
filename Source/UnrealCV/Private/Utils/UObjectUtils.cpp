#include "UnrealCVPrivate.h"
#include "UObjectUtils.h"

TArray<AActor*> GetActorPtrList(UWorld* World)
{
	TArray<UObject*> UObjectList;
	bool bIncludeDerivedClasses = true;
	EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject;
	EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::AllFlags;
	GetObjectsOfClass(AActor::StaticClass(), UObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);

	TArray<AActor*> ActorList;
	for (UObject* ActorObject : UObjectList)
	{
		AActor* Actor = Cast<AActor>(ActorObject);
		if (Actor->GetWorld() != World) continue;
		if (ActorList.Contains(Actor) == false)
		{
			ActorList.Add(Actor);
		}
	}
	return ActorList;
}

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
