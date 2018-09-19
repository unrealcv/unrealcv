// Weichao Qiu @ 2017
// Attach two lit sensor and two depth sensor components

#include "StereoCameraActor.h"

AStereoCameraActor::AStereoCameraActor()
	: Super(), BaselineDistance(80)
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	// Make the scene component the root component
	RootComponent = SceneComponent;

	LeftSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("LeftSensor"));
	LeftSensor->SetupAttachment(SceneComponent);

	RightSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("RightSensor"));
	RightSensor->SetupAttachment(SceneComponent);

	SetBaselineDistance();
}

void AStereoCameraActor::SetBaselineDistance()
{
	LeftSensor->SetRelativeLocation(FVector(0, - BaselineDistance / 2, 0));
	RightSensor->SetRelativeLocation(FVector(0, + BaselineDistance / 2, 0));
}

void AStereoCameraActor::BeginPlay()
{
	Super::BeginPlay(); // Enable ticking actor
}

TArray<FString> AStereoCameraActor::GetSensorNames()
{
	return { 
		this->GetName() + TEXT("_") + LeftSensor->GetName(),
		this->GetName() + TEXT("_") + RightSensor->GetName(),
	};
}

TArray<UFusionCamSensor*> AStereoCameraActor::GetSensors()
{
	return {
		this->LeftSensor,
		this->RightSensor,
	};
}


#if WITH_EDITOR
void AStereoCameraActor::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AStereoCameraActor, BaselineDistance))
	{
		SetBaselineDistance();
	}
}
#endif