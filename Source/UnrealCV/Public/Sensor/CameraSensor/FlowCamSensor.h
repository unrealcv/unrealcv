// Weichao Qiu @ 2017
#pragma once

#include "BaseCameraSensor.h"
#include "FlowCamSensor.generated.h"

/* Current render process is based on ESceneCaptureSource defined in UE5,
 * which supports color(rgb), depth and normal image but not optical flow image.
 * So, to render optical flow image, we have to define a special optical flow material
 * like '/UnrealCV/OpticalFlowMaterial', which is used to extract motion vectors.
 *
 * depth, normal pipeline: (PostProcess material - world level)
 *   PlayerViewMode.h/.cpp (set world level PostProcess material)
 *   BaseCameraSensor.h/.cpp (PostProcess material application)
 *   DepthCamSensor/NormalCamSensor (specific capture logic)
 *
 * Segmentation pipeline: (mesh level material)
 *   ObjectAnnotator.h/.cpp (AnnotationComponent manager, to create/update)
 *   AnnotationComponent.h/.cpp (component attached to the mesh)
 *   AnnotationCamSensor.h/.cpp (only to render)
 *
 * Optical flow pipeline: (PostProcess material - world level) ✅ IMPLEMENTED
 *   PlayerViewMode.h/.cpp (set world level PostProcess material)
 *   FlowCamSensor.h/.cpp (optical flow capture with PostProcess material)
 *   FusionCamSensor.h/.cpp (integration point)
 */

UCLASS(meta = (BlueprintSpawnableComponent))
class UNREALCV_API UFlowCamSensor : public UBaseCameraSensor
{
	GENERATED_BODY()

public:
	UFlowCamSensor(const FObjectInitializer& ObjectInitializer);

	virtual void InitTextureTarget(int FilmWidth, int FilmHeight) override;
	// LogOutputDevice: Error: Ensure condition failed: false  [File:D:\build\++UE5\Sync\Engine\Source\Runtime\Engine\Private\Components\SceneComponent.cpp] [Line: 2104] 
	// LogOutputDevice: Error: Template Mismatch during attachment. Attaching instanced component to template component. Parent 'FusionCamSensor_GEN_VARIABLE' (Owner 'None') Self 'FusionCamSensor_GEN_VARIABLE_FlowCamSensor' (Owner 'BP_Drone01_C_1').

	void CaptureFlow(TArray<FColor>& Image, int& Width, int& Height);

private:
	/** Post process material to extract optical flow data */
	// UPROPERTY()
	UMaterial* OpticalFlowPPMaterial;
};
