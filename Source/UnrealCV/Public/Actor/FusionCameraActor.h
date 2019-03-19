// Weichao Qiu @ 2017
#pragma once

#include "FusionCamSensor.h"
#include "Actor/CamSensorActor.h"
#include "FusionCameraActor.generated.h"

UCLASS()
class UNREALCV_API AFusionCameraActor : public ACamSensorActor
{
	GENERATED_BODY()

public:
	AFusionCameraActor();

	// This can be extended to support multiple cameras
	virtual TArray<FString> GetSensorNames();

	virtual TArray<UFusionCamSensor*> GetSensors();

private:
	// Define it to be VisibleAnywhere not EditableAnywhere. This is enough for changing the component property
	UPROPERTY(Category = AFusionCameraActor, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UFusionCamSensor* FusionCamSensor;
};
