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

	static void GetAnnotationComponents(UWorld* World, TArray<TWeakObjectPtr<UPrimitiveComponent> >& ComponentList);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction * T);

	void CaptureSeg(TArray<FColor>& ImageData, int& Width, int& Height);

	void InitTextureTarget(int FilmWidth, int FilmHeight);
};
