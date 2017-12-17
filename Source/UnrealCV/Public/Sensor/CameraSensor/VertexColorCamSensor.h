// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "VertexColorCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UVertexColorCamSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    UVertexColorCamSensor(const FObjectInitializer& ObjectInitializer);

    virtual void OnRegister() override;
};
