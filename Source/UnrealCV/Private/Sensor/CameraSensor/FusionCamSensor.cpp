#include "UnrealCVPrivate.h"
#include "FusionCamSensor.h"
#include "ImageUtil.h"
#include "Serialization.h"

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
	LitCamSensor = CreateDefaultSubobject<ULitCamSensor>("LitCamSensor");
	// LitCamSensor = NewObject<ULitCamSensor>(this, "LitCamSensorVersion1");
	// NewObject will create an error showing graph linked to private property when saving the map.
	// Use SetupAttachment in CTOR
	// LitCamSensor->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
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

	// VertexColorCamSensor = CreateDefaultSubobject<UVertexColorCamSensor>("VertexColorCamSensor");
	// FusionSensors.Add(VertexColorCamSensor);

	StencilCamSensor = CreateDefaultSubobject<UStencilCamSensor>("StencilCamSensor");
	FusionSensors.Add(StencilCamSensor);

	AnnotationCamSensor = CreateDefaultSubobject<UAnnotationCamSensor>("AnnotationCamSensor");
	FusionSensors.Add(AnnotationCamSensor);

	LitSlowCamSensor = CreateDefaultSubobject<ULitSlowCamSensor>("LitSlowCamSensor");
	FusionSensors.Add(LitSlowCamSensor);

	PreviewCamera = CreateDefaultSubobject<UCameraComponent>("PreviewCamera");
	PreviewCamera->SetupAttachment(this);

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

}

void UFusionCamSensor::OnRegister()
{
	Super::OnRegister();

	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (IsValid(Sensor))
		{
			Sensor->RegisterComponent();
		}
		else
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Invalid sensor is found in the OnRegister of FusionCamSensor"));
		}
	}
}

bool UFusionCamSensor::GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut)
{
	// From CameraComponent
	if (bIsActive)
	{
		GetCameraView(DeltaTime, ViewOut);
	}
	return bIsActive;
}

void UFusionCamSensor::GetLit(TArray<FColor>& LitData, int& Width, int& Height)
{
	this->LitCamSensor->Capture(LitData, Width, Height);
}

void UFusionCamSensor::GetLitSlow(TArray<FColor>& LitData, int& Width, int& Height)
{
	this->LitSlowCamSensor->Capture(LitData, Width, Height);
}

void UFusionCamSensor::GetDepth(TArray<float>& DepthData, int& Width, int& Height)
{
	this->DepthCamSensor->CaptureDepth(DepthData, Width, Height);
}

void UFusionCamSensor::GetPlaneDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height)
{
	this->PlaneDepthCamSensor->CaptureDepth(DepthData, Width, Height);
}

void UFusionCamSensor::GetVisDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height)
{
	this->VisDepthCamSensor->CaptureDepth(DepthData, Width, Height);
}

void UFusionCamSensor::GetNormal(TArray<FColor>& NormalData, int& Width, int& Height)
{
	this->NormalCamSensor->Capture(NormalData, Width, Height);
}

void UFusionCamSensor::GetObjectMask(TArray<FColor>& ObjMaskData, int& Width, int& Height)
{
	this->AnnotationCamSensor->Capture(ObjMaskData, Width, Height);
}

void UFusionCamSensor::GetVertexColor(TArray<FColor>& VertexColorData, int& Width, int& Height)
{
	this->VertexColorCamSensor->Capture(VertexColorData, Width, Height);
}

void UFusionCamSensor::GetStencil(TArray<FColor>& StencilData, int& Width, int& Height)
{
	this->StencilCamSensor->Capture(StencilData, Width, Height);
}
