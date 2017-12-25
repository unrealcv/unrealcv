// Weichao Qiu @ 2017

#pragma once

#include "BaseCameraSensor.h"
#include "NormalCamSensor.generated.h"

/** Surface normal sensor */
UCLASS(meta = (BlueprintSpawnableComponent))
class UNormalCamSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    UNormalCamSensor(const FObjectInitializer& ObjectInitializer);

    virtual void OnRegister() override;

private:
    /** Post process material to extract surface normal data */
    UPROPERTY()
    UMaterial* SurfaceNormalPPMaterial;
};
