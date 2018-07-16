// Weichao Qiu @ 2018
#pragma once

#include "BaseCameraSensor.h"
#include "LitSlowCamSensor.generated.h"

/** RGB color sensor */
// UCLASS(meta = (BlueprintSpawnableComponent))
UCLASS()
class ULitSlowCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	ULitSlowCamSensor(const FObjectInitializer& ObjectInitializer);

	// UFUNCTION(BlueprintPure, Category = "unrealcv")
	virtual void Capture(TArray<FColor>& ImageData, int& Width, int& Height) override;

	virtual void SetupRenderTarget() override;
};
