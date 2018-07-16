// Weichao Qiu @ 2018
#include "FusionCamSensor.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "ImageUtil.h"
#include "Serialization.h"
#include "UnrealcvLog.h"
#include "UnrealcvServer.h"

// Sensors included in FusionSensor
#include "LitCamSensor.h"
#include "DepthCamSensor.h"
#include "VertexColorCamSensor.h"
#include "NormalCamSensor.h"
#include "StencilCamSensor.h"
#include "AnnotationCamSensor.h"
#include "PlaneDepthCamSensor.h"
#include "VisDepthCamSensor.h"
#include "NontransDepthCamSensor.h"
#include "LitSlowCamSensor.h"

UFusionCamSensor::UFusionCamSensor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PreviewCamera = CreateDefaultSubobject<UCameraComponent>("PreviewCamera");
	PreviewCamera->SetupAttachment(this);

	LitCamSensor = CreateDefaultSubobject<ULitCamSensor>("LitCamSensor");
	FusionSensors.Add(LitCamSensor);
	DepthCamSensor = CreateDefaultSubobject<UDepthCamSensor>("DepthCamSensor");
	FusionSensors.Add(DepthCamSensor);
	VisDepthCamSensor = CreateDefaultSubobject<UVisDepthCamSensor>("VisDepthCamSensor");
	FusionSensors.Add(VisDepthCamSensor);
	PlaneDepthCamSensor = CreateDefaultSubobject<UPlaneDepthCamSensor>("PlaneDepthCamSensor");
	FusionSensors.Add(PlaneDepthCamSensor);
	NormalCamSensor = CreateDefaultSubobject<UNormalCamSensor>("NormalCamSensor");
	FusionSensors.Add(NormalCamSensor);
	NontransDepthCamSensor = CreateDefaultSubobject<UNontransDepthCamSensor>("NontransDepthCamSensor");
	FusionSensors.Add(NontransDepthCamSensor);
	StencilCamSensor = CreateDefaultSubobject<UStencilCamSensor>("StencilCamSensor");
	FusionSensors.Add(StencilCamSensor);
	AnnotationCamSensor = CreateDefaultSubobject<UAnnotationCamSensor>("AnnotationCamSensor");
	FusionSensors.Add(AnnotationCamSensor);
	LitSlowCamSensor = CreateDefaultSubobject<ULitSlowCamSensor>("LitSlowCamSensor");
	FusionSensors.Add(LitSlowCamSensor);


	FServerConfig& Config = FUnrealcvServer::Get().Config;
	FilmWidth = Config.Width == 0 ? 640 : Config.Width;
	FilmHeight = Config.Height == 0 ? 480 : Config.Height; 

	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (IsValid(Sensor))
		{
			Sensor->SetupAttachment(this);
		}
		else
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Invalid sensor is found in the ctor of FusionCamSensor"));
		}
	}

	// this->FOVAngle = Config.FOV;
	// SetFilmSize(FilmWidth, FilmHeight); // This should not not be done in CTOR.
}

void UFusionCamSensor::BeginPlay()
{
	Super::BeginPlay();

	for (UBaseCameraSensor* Sensor: FusionSensors)
	{
		if (IsValid(Sensor))
		{
			Sensor->FilmWidth = FilmWidth;
			Sensor->FilmHeight = FilmHeight;
		}
	}
}

// void UFusionCamSensor::OnRegister()
// {
// 	Super::OnRegister();

// 	for (UBaseCameraSensor* Sensor : FusionSensors)
// 	{
// 		if (IsValid(Sensor))
// 		{
// 			Sensor->RegisterComponent();
// 		}
// 		else
// 		{
// 			UE_LOG(LogUnrealCV, Warning, TEXT("Invalid sensor is found in the OnRegister of FusionCamSensor"));
// 		}
// 	}
// }

bool UFusionCamSensor::GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut)
{
	// From CameraComponent
	if (bIsActive)
	{
		this->LitCamSensor->GetCameraView(DeltaTime, ViewOut);
	}
	return bIsActive;
}

void UFusionCamSensor::GetLit(TArray<FColor>& LitData, int& Width, int& Height, ELitMode LitMode)
{
	switch (LitMode)
	{
		case ELitMode::Lit:
			this->LitCamSensor->Capture(LitData, Width, Height);
			break;
		case ELitMode::Slow:
			this->LitSlowCamSensor->Capture(LitData, Width, Height);
			break;
	}
}

// void UFusionCamSensor::GetLitSlow(TArray<FColor>& LitData, int& Width, int& Height)
// {
// 	this->LitSlowCamSensor->Capture(LitData, Width, Height);
// }

void UFusionCamSensor::GetDepth(TArray<float>& DepthData, int& Width, int& Height, EDepthMode DepthMode)
{
	this->DepthCamSensor->CaptureDepth(DepthData, Width, Height);
}

// void UFusionCamSensor::GetPlaneDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height)
// {
// 	this->PlaneDepthCamSensor->CaptureDepth(DepthData, Width, Height);
// }

// void UFusionCamSensor::GetVisDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height)
// {
// 	this->VisDepthCamSensor->CaptureDepth(DepthData, Width, Height);
// }

void UFusionCamSensor::GetNormal(TArray<FColor>& NormalData, int& Width, int& Height)
{
	this->NormalCamSensor->Capture(NormalData, Width, Height);
}

void UFusionCamSensor::GetSeg(TArray<FColor>& ObjMaskData, int& Width, int& Height, ESegMode SegMode)
{
	this->AnnotationCamSensor->Capture(ObjMaskData, Width, Height);
}

// void UFusionCamSensor::GetVertexColor(TArray<FColor>& VertexColorData, int& Width, int& Height)
// {
// 	this->VertexColorCamSensor->Capture(VertexColorData, Width, Height);
// }

// void UFusionCamSensor::GetStencil(TArray<FColor>& StencilData, int& Width, int& Height)
// {
// 	this->StencilCamSensor->Capture(StencilData, Width, Height);
// }

FVector UFusionCamSensor::GetSensorLocation()
{
	return this->GetComponentLocation(); // World space
}

FRotator UFusionCamSensor::GetSensorRotation()
{
	return this->GetComponentRotation(); // World space
}

void UFusionCamSensor::SetSensorLocation(FVector Location)
{
	this->SetWorldLocation(Location);
}

void UFusionCamSensor::SetSensorRotation(FRotator Rotator)
{
	this->SetWorldRotation(Rotator);
}

void UFusionCamSensor::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UFusionCamSensor, PresetFilmSize))
	{
		switch(PresetFilmSize)
		{
		case EPresetFilmSize::F640x480:
			FilmWidth = 640;
			FilmHeight = 480;
			break;
		case EPresetFilmSize::F1080p:
			FilmWidth = 1920;
			FilmHeight = 1080;
			break;
		case EPresetFilmSize::F720p:
			FilmWidth = 1280;
			FilmHeight = 720;
			break;
		}
	}
}
