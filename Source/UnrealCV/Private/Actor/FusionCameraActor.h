// Weichao Qiu @ 2017
#pragma once

#include "FusionCamSensor.h"
#include "FusionCameraActor.generated.h"

UCLASS()
class AFusionCameraActor : public AActor
{
	GENERATED_BODY()

public:
	AFusionCameraActor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Category = AFusionCameraActor, VisibleAnywhere, BlueprintReadOnly)
	UFusionCamSensor* FusionCamSensor;
};
