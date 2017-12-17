// Weichao Qiu @ 2017
#pragma once
#include "BaseCameraSensor.h"
#include "StencilCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UStencilCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	UStencilCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void OnRegister() override;

private:
	UPROPERTY()
	UMaterial* StencilPPMaterial;
};
