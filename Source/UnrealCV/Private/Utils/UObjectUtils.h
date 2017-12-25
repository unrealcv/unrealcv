// Weichao Qiu @ 2017
// UObject related utility functions, based on the reflection system of UE4

#pragma once


TArray<AActor*> GetActorPtrList(UWorld* World);

AActor* GetActorById(UWorld* World, FString ActorId);

UObject* GetObjectById(UWorld* World, FString ObjectId);
