// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "ObjMaskCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UObjMaskCamSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    UObjMaskCamSensor(const FObjectInitializer& ObjectInitializer);

    virtual void OnRegister() override;
};
