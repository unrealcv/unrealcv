// Weichao Qiu @ 2017
#pragma once

#include "FusionCamSensor.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "FusionCameraActor.generated.h"

UCLASS()
class UNREALCV_API AFusionCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AFusionCameraActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Category = AFusionCameraActor, VisibleAnywhere, BlueprintReadOnly)
	UFusionCamSensor* FusionCamSensor;
};
