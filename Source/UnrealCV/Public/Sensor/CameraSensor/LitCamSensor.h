// Weichao Qiu @ 2017

#pragma once

#include "BaseCameraSensor.h"
#include "LitCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class ULitSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    ULitSensor(const FObjectInitializer& ObjectInitializer);

private:
    UPROPERTY()
    UMaterial* LitPPMaterial;
};
