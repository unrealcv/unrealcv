// Weichao Qiu @ 2017

#pragma once
#include "BaseCameraSensor.h"
#include "DepthCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UDepthCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	UDepthCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual	void OnRegister() override;

	void Capture(TArray<FFloat16Color>& DepthData, int& Width, int& Height);
};
