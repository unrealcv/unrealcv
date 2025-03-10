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
class UNREALCV_API ULitCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	ULitCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void InitTextureTarget(int FilmWidth, int FilmHeight) override;

	void CaptureLit(TArray<FColor>& Image, int& Width, int& Height);
	void CaptureLit_Updated(TArray<FColor>& Image, int& Width, int& Height);
};
