// Weichao Qiu @ 2017
#pragma once
#include "BaseCameraSensor.h"
#include "VisDepthCamSensor.generated.h"

/** VisDepth sensor */
UCLASS(meta = (BlueprintSpawnableComponent))
class UVisDepthCamSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    UVisDepthCamSensor(const FObjectInitializer& ObjectInitializer);

    virtual void OnRegister() override;

    void CaptureDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height);
};
