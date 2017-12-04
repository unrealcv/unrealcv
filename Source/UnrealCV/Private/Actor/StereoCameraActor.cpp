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

	LeftLitSensor = CreateDefaultSubobject<ULitSensor>(TEXT("Left Lit"));
	LeftLitSensor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RightLitSensor = CreateDefaultSubobject<ULitSensor>(TEXT("Right Lit"));
	RightLitSensor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	LeftObjMaskSensor = CreateDefaultSubobject<UObjMaskSensor>(TEXT("Left Object Mask"));
	LeftObjMaskSensor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
	RightObjMaskSensor = CreateDefaultSubobject<UObjMaskSensor>(TEXT("Right Object Mask"));
	RightObjMaskSensor->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);

	int Baseline = 100;

	LeftLitSensor->SetRelativeLocation (FVector(0, - Baseline / 2, 0));
	RightLitSensor->SetRelativeLocation(FVector(0, + Baseline / 2, 0));

	LeftObjMaskSensor->SetRelativeLocation (FVector(0, - Baseline / 2, 0));
	RightObjMaskSensor->SetRelativeLocation(FVector(0, + Baseline / 2, 0));

}

void AStereoCameraActor::BeginPlay()
{
	Super::BeginPlay(); // Enable ticking actor

	FVertexColorPainter VertexColorPainter;
	// Paint all objects
	for (TActorIterator<AStaticMeshActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		// Same as with the Object Iterator, access the subclass instance with the * or -> operators.
		AStaticMeshActor *MeshActor = *ActorItr;
		VertexColorPainter.PaintVertexColor(MeshActor, FColor::MakeRandomColor());
	}
}

void AStereoCameraActor::Tick(float DeltaSeconds)
{
	FString LeftLitFilename = UVisionBP::FormatFrameFilename(TEXT("C:/temp/left_lit_%d.bmp"));
	FString RightLitFilename = UVisionBP::FormatFrameFilename(TEXT("C:/temp/right_lit_%d.bmp"));
	FString LeftObjMaskFilename = UVisionBP::FormatFrameFilename(TEXT("C:/temp/left_obj_%d.bmp"));
	FString RightObjMaskFilename = UVisionBP::FormatFrameFilename(TEXT("C:/temp/right_obj_%d.bmp"));

	LeftLitSensor->GetLitAsync(LeftLitFilename);
	RightLitSensor->GetLitAsync(RightLitFilename);
	LeftObjMaskSensor->GetLitAsync(LeftObjMaskFilename);
	RightObjMaskSensor->GetLitAsync(RightObjMaskFilename);

}
