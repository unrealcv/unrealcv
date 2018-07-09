// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "LitCamSensor.generated.h"

/** 
 * RGB color sensor 
 * The alias issue was reported here
 * https://forums.unrealengine.com/development-discussion/rendering/59403-scenecapturecomponent2d-antialiasing
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class ULitCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	ULitCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void OnRegister() override;

private:
	UPROPERTY()
	UMaterial* LitPPMaterial;
};
