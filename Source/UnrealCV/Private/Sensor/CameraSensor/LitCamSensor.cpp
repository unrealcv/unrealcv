#include "UnrealCVPrivate.h"
#include "LitCamSensor.h"


ULitSensor::ULitSensor(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer)
{
	FString LitPPMaterialPath = TEXT("Material'/ue4_human/PostProcessMaterial/LitPPM.LitPPM'");
	ConstructorHelpers::FObjectFinder<UMaterial> Material(*LitPPMaterialPath);

	LitPPMaterial = Material.Object;
	// ConsturctorHelpers is only available for UObject.
	SetPostProcessMaterial(LitPPMaterial);
	SetFOV(90);

	Width = 1024;
	Height = 768;
}
