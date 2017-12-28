// Weichao Qiu @ 2017
#pragma once
#include "BaseCameraSensor.h"
#include "PlaneDepthCamSensor.generated.h"

/** PlaneDepth sensor */
UCLASS(meta = (BlueprintSpawnableComponent))
class UPlaneDepthCamSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    UPlaneDepthCamSensor(const FObjectInitializer& ObjectInitializer);

    virtual void OnRegister() override;

    void CaptureDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height);
};
