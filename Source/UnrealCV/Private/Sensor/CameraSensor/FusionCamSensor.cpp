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

UFusionCamSensor::UFusionCamSensor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LitCamSensor = CreateDefaultSubobject<ULitCamSensor>("LitCamSensor");
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

	VertexColorCamSensor = CreateDefaultSubobject<UVertexColorCamSensor>("VertexColorCamSensor");
	FusionSensors.Add(VertexColorCamSensor);

	StencilCamSensor = CreateDefaultSubobject<UStencilCamSensor>("StencilCamSensor");
	FusionSensors.Add(StencilCamSensor);

	AnnotationCamSensor = CreateDefaultSubobject<UAnnotationCamSensor>("AnnotationCamSensor");
	FusionSensors.Add(AnnotationCamSensor);

	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (IsValid(Sensor))
		{
			Sensor->SetupAttachment(this);
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
	}
}

void UFusionCamSensor::GetLit(TArray<FColor>& LitData, int& Width, int& Height)
{
	// Correct, but slow
	// this->LitCamSensor->CaptureSlow(LitData, Width, Height);
	this->LitCamSensor->Capture(LitData, Width, Height);
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
