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

UFusionCamSensor::UFusionCamSensor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LitCamSensor = CreateDefaultSubobject<ULitCamSensor>("LitCamSensor");
	// LitCamSensor->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	LitCamSensor->SetupAttachment(this);
	DepthCamSensor = CreateDefaultSubobject<UDepthCamSensor>("DepthCamSensor");
	DepthCamSensor->SetupAttachment(this);
	NormalCamSensor = CreateDefaultSubobject<UNormalCamSensor>("NormalCamSensor");
	NormalCamSensor->SetupAttachment(this);
	ObjMaskCamSensor = CreateDefaultSubobject<UVertexColorCamSensor>("ObjMaskCamSensor");
	ObjMaskCamSensor->SetupAttachment(this);
	StencilCamSensor = CreateDefaultSubobject<UStencilCamSensor>("StencilCamSensor");
	StencilCamSensor->SetupAttachment(this);
}

void UFusionCamSensor::OnRegister()
{
	Super::OnRegister();

	if (LitCamSensor) LitCamSensor->RegisterComponent();
	if (DepthCamSensor) DepthCamSensor->RegisterComponent();
	if (NormalCamSensor) NormalCamSensor->RegisterComponent();
	if (ObjMaskCamSensor) ObjMaskCamSensor->RegisterComponent();
	if (StencilCamSensor) StencilCamSensor->RegisterComponent();
}

void UFusionCamSensor::GetLit(TArray<FColor>& LitData, int& Width, int& Height)
{
	// Correct, but slow
	// this->LitCamSensor->CaptureSlow(LitData, Width, Height);
	this->LitCamSensor->Capture(LitData, Width, Height);
}

void UFusionCamSensor::GetDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height)
{
	this->DepthCamSensor->CaptureDepth(DepthData, Width, Height);
}

void UFusionCamSensor::GetNormal(TArray<FColor>& NormalData, int& Width, int& Height)
{
	this->NormalCamSensor->Capture(NormalData, Width, Height);
}

void UFusionCamSensor::GetObjectMask(TArray<FColor>& ObjMaskData, int& Width, int& Height)
{
	this->ObjMaskCamSensor->Capture(ObjMaskData, Width, Height);
}

void UFusionCamSensor::GetStencil(TArray<FColor>& StencilData, int& Width, int& Height)
{
	this->StencilCamSensor->Capture(StencilData, Width, Height);
}
