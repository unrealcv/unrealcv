// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "NormalCamSensor.generated.h"

/** Surface normal sensor */
UCLASS()
class UNormalCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	UNormalCamSensor(const FObjectInitializer& ObjectInitializer);

private:
	/** Post process material to extract surface normal data */
	UPROPERTY()
	UMaterial* SurfaceNormalPPMaterial;
};
