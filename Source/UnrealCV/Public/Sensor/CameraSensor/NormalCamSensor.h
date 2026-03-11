// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "NormalCamSensor.generated.h"

/** Surface normal sensor */
UCLASS()
class UNREALCV_API UNormalCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	UNormalCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void InitTextureTarget(int filmWidth, int filmHeight) override;

	void CaptureNormal(TArray<FColor>& ImageData, int& Width, int& Height);

private:
	/** Post process material to extract surface normal data */
	UPROPERTY()
	UMaterial* SurfaceNormalPPMaterial;
};
