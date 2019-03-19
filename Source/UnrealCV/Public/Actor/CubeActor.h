// Weichao Qiu @ 2019
#pragma once

#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Engine/StaticMeshActor.h"
#include "CubeActor.generated.h"


/**
  Generate a simple cube for debug dynamic object creation
 */
UCLASS()
class UNREALCV_API ACubeActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	ACubeActor();

};
