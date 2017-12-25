// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "VertexColorCamSensor.generated.h"

/** Vertex color sensor, can be used for object mask for UE4 <= 4.16 */
UCLASS(meta = (BlueprintSpawnableComponent))
class UVertexColorCamSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    UVertexColorCamSensor(const FObjectInitializer& ObjectInitializer);

    virtual void Capture(TArray<FColor>& ImageData, int& Width, int& Height) override;

    virtual void OnRegister() override;
};
