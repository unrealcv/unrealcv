// Weichao Qiu @ 2017
#pragma once

#include "Runtime/Engine/Classes/GameFramework/Actor.h"

class UNREALCV_API FVertexSensor
{
public:
	FVertexSensor(const AActor* InActor);

	TArray<FVector> GetVertexArray();

private:
	const AActor* OwnerActor;
};
