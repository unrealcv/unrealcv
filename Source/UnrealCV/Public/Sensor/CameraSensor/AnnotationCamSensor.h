// Weichao Qiu @ 2017

#pragma once

#include "BaseCameraSensor.h"
#include "AnnotationCamSensor.generated.h"

/** Annotation sensor, utilize the UAnnnotationComponent */
UCLASS(meta = (BlueprintSpawnableComponent))
class UAnnotationCamSensor : public UBaseCameraSensor
{
    GENERATED_BODY()

public:
    UAnnotationCamSensor(const FObjectInitializer& ObjectInitializer);

    virtual void OnRegister() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T);

	virtual void Capture(TArray<FColor>& ImageData, int& Width, int& Height) override;
};
