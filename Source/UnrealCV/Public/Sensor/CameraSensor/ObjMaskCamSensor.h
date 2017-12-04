// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "ObjMaskCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UObjMaskSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    UObjMaskSensor(const FObjectInitializer& ObjectInitializer);

};
