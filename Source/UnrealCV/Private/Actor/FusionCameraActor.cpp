// Weichao Qiu @ 2017

#include "FusionCameraActor.h"

AFusionCameraActor::AFusionCameraActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	FusionCamSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("FusionCameraSensor"));
	RootComponent = FusionCamSensor;
}
