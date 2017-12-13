// Weichao Qiu @ 2017
// Attach two lit sensor and two depth sensor components

#include "UnrealCVPrivate.h"
#include "StereoCameraActor.h"
#include "VertexColorPainter.h"
#include "VisionBP.h"

AStereoCameraActor::AStereoCameraActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));

	// Make the scene component the root component
	// RootComponent = SceneComponent;
	PrimaryActorTick.bCanEverTick = true;
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;

	LeftSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("Left Sensor"));
	LeftSensor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RightSensor = CreateDefaultSubobject<UFusionCamSensor>(TEXT("Right Sensor"));
	RightSensor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	int Baseline = 100;

	LeftSensor->SetRelativeLocation (FVector(0, - Baseline / 2, 0));
	RightSensor->SetRelativeLocation(FVector(0, + Baseline / 2, 0));
}

void AStereoCameraActor::BeginPlay()
{
	Super::BeginPlay(); // Enable ticking actor
}

void AStereoCameraActor::Tick(float DeltaSeconds)
{
	FString LeftLitFilename = UVisionBP::FormatFrameFilename(TEXT("C:/temp/left_lit_%d.bmp"));
	FString RightLitFilename = UVisionBP::FormatFrameFilename(TEXT("C:/temp/right_lit_%d.bmp"));
	FString LeftObjMaskFilename = UVisionBP::FormatFrameFilename(TEXT("C:/temp/left_obj_%d.bmp"));
	FString RightObjMaskFilename = UVisionBP::FormatFrameFilename(TEXT("C:/temp/right_obj_%d.bmp"));

	//LeftSensor->GetLitFilename(LeftLitFilename);
	//RightSensor->GetLitFilename(RightLitFilename);
	//LeftSensor->GetObjectMaskFilename(LeftObjMaskFilename);
	//RightSensor->GetObjectMaskFilename(RightObjMaskFilename);
}
