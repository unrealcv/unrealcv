// Weichao Qiu @ 2017

#include "FusionCameraActor.h"

AFusionCameraActor::AFusionCameraActor()
	: Super()
{
	PrimaryActorTick.bCanEverTick = true;

	FusionCamSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("FusionCameraSensor"));
	RootComponent = FusionCamSensor;
}

TArray<FString> AFusionCameraActor::GetSensorNames()
{
	return { this->GetName() };
}

TArray<UFusionCamSensor*> AFusionCameraActor::GetSensors()
{
	return { this->FusionCamSensor };
}