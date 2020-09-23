// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "DepthCamSensor.generated.h"

/** Depth sensor */
UCLASS(meta = (BlueprintSpawnableComponent))
class UDepthCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	UDepthCamSensor(const FObjectInitializer& ObjectInitializer);

	void CaptureDepth(TArray<float>& DepthData, int& Width, int& Height);

	virtual void InitTextureTarget(int FilmWidth, int FilmHeight) override;

	UPROPERTY(EditInstanceOnly, Category = "unrealcv")
	bool bIgnoreTransparentObjects;
};
