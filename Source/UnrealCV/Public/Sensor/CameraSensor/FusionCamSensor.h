// Weichao Qiu @ 2017

#include "BaseCameraSensor.h"
#include "FusionCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UFusionCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()
public:

	UFusionCamSensor(const FObjectInitializer& ObjectInitializer);

private:
};
