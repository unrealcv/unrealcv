// Weichao Qiu @ 2017
#pragma once

#include "FusionCamSensor.h"
#include "PawnCamSensor.generated.h"

/** Sensor to be attached to the player's view */
UCLASS(meta = (BlueprintSpawnableComponent))
class UPawnCamSensor : public UFusionCamSensor
{
	GENERATED_BODY()

public:
	UPawnCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T);

	// virtual void SetSensorLocation(FVector Location) override;
	// virtual void SetSensorRotation(FRotator Rotation) override;
	void SetSensorLocation(FVector Location);
	void SetSensorRotation(FRotator Rotation);
};
