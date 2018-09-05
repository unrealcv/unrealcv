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


	virtual int GetSensorNum() { return GetSensors().Num(); }

	// This can be extended to support multiple cameras
	virtual TArray<FString> GetSensorNames();

	virtual TArray<UFusionCamSensor*> GetSensors();

private:
	// Define it to be VisibleAnywhere not EditableAnywhere. This is enough for changing the component property
	UPROPERTY(Category = AFusionCameraActor, VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	UFusionCamSensor* FusionCamSensor;
};
