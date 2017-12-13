#include "UnrealCVPrivate.h"
#include "FusionCamSensor.h"
#include "ImageUtil.h"
#include "Serialization.h"
// Sensors included in FusionSensor
#include "LitCamSensor.h"
#include "DepthCamSensor.h"
#include "ObjMaskCamSensor.h"
#include "NormalCamSensor.h"

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
	ObjMaskCamSensor = CreateDefaultSubobject<UObjMaskCamSensor>("ObjMaskCamSensor");
	ObjMaskCamSensor->SetupAttachment(this);
}

void UFusionCamSensor::OnRegister()
{
	Super::OnRegister();

	LitCamSensor->RegisterComponent();
	DepthCamSensor->RegisterComponent();
	NormalCamSensor->RegisterComponent();
	ObjMaskCamSensor->RegisterComponent();
}

void UFusionCamSensor::GetLit(TArray<FColor>& LitData, int& Width, int& Height)
{
	// Correct, but slow
	// this->LitCamSensor->CaptureSlow(LitData, Width, Height);
	this->LitCamSensor->Capture(LitData, Width, Height);
}

void UFusionCamSensor::GetDepth(TArray<FFloat16Color>& DepthData, int& Width, int& Height)
{
	this->DepthCamSensor->Capture(DepthData, Width, Height);
}

void UFusionCamSensor::GetNormal(TArray<FColor>& NormalData, int& Width, int& Height)
{
	this->NormalCamSensor->CaptureSlow(NormalData, Width, Height);
}

void UFusionCamSensor::GetObjectMask(TArray<FColor>& ObjMaskData, int& Width, int& Height)
{
	this->ObjMaskCamSensor->CaptureSlow(ObjMaskData, Width, Height);
}
