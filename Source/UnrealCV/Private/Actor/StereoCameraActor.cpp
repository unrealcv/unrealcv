// Weichao Qiu @ 2017
// Attach two lit sensor and two depth sensor components

#include "UnrealCVPrivate.h"
#include "StereoCameraActor.h"
#include "VertexColorPainter.h"
#include "VisionBP.h"

AStereoCameraActor::AStereoCameraActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), BaseLineDistance(80)
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	// Make the scene component the root component
	RootComponent = SceneComponent;

	LeftSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("LeftSensor"));
	LeftSensor->SetupAttachment(SceneComponent);

	RightSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("RightSensor"));
	RightSensor->SetupAttachment(SceneComponent);

	LeftSensor->SetRelativeLocation (FVector(0, - BaseLineDistance / 2, 0));
	RightSensor->SetRelativeLocation(FVector(0, + BaseLineDistance / 2, 0));
}

void AStereoCameraActor::BeginPlay()
{
	Super::BeginPlay(); // Enable ticking actor
}
