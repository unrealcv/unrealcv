// Weichao Qiu @ 2017
// UObject related utility functions, based on the reflection system of UE4
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"

AActor* GetActorById(UWorld* World, FString ActorId);

UObject* GetObjectById(UWorld* World, FString ObjectId);
