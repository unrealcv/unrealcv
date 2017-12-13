// Weichao Qiu @ 2017

#include "UnrealCVPrivate.h"
#include "FusionCameraActor.h"

AFusionCameraActor::AFusionCameraActor(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    FusionCamSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("Fusion Camera Sensor"));
    RootComponent = FusionCamSensor;
}
