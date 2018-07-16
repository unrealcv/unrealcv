// Weichao Qiu @ 2018

#pragma once

#include "BaseCameraSensor.h"
#include "NontransDepthCamSensor.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class UNontransDepthCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	UNontransDepthCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void SetupRenderTarget() override;

	virtual void CaptureDepth(TArray<float>& DepthData, int& Width, int& Height);
};
