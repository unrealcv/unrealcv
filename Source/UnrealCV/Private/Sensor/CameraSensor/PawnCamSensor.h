// Weichao Qiu @ 2017
#pragma once

#include "FusionCamSensor.h"
#include "PawnCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UPawnCamSensor : public UFusionCamSensor
{
	GENERATED_BODY()

public:
	UPawnCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T);
};
