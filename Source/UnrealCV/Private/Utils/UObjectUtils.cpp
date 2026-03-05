// Weichao Qiu @ 2018
#include "UObjectUtils.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectIterator.h"

AActor* GetActorById(UWorld* World, FString ActorId)
{
	if (!World)
	{
		return nullptr;
	}

	static TWeakObjectPtr<UWorld> CachedWorld;
	static TMap<FString, TWeakObjectPtr<AActor>> ActorCache;

	if (CachedWorld.Get() != World)
	{
		ActorCache.Empty();
		for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
		{
			AActor* Actor = *ActorItr;
			if (Actor && Actor->GetWorld() == World)
			{
				ActorCache.Add(Actor->GetName(), Actor);
			}
		}
		CachedWorld = World;
	}

	if (TWeakObjectPtr<AActor>* CachedActor = ActorCache.Find(ActorId))
	{
		if (CachedActor->IsValid())
		{
			AActor* Actor = CachedActor->Get();
			if (Actor && Actor->GetName() == ActorId)
			{
				return Actor;
			}
		}
		ActorCache.Remove(ActorId);
	}

	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (Actor->GetWorld() == World && Actor->GetName() == ActorId)
		{
			ActorCache.Add(ActorId, Actor);
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
