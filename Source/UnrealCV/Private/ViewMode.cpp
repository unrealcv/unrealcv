#include "UnrealCVPrivate.h"
#include "ViewMode.h"
#include "BufferVisualizationData.h"

void FViewMode::Depth(FEngineShowFlags& ShowFlags)
{
	// SetCurrentBufferVisualizationMode(TEXT("SceneDepthWorldUnits"));
}

void FViewMode::VisDepth(FEngineShowFlags& ShowFlags)
{
	// SetCurrentBufferVisualizationMode(TEXT("SceneDepth"));
}

void FViewMode::Lit(FEngineShowFlags& ShowFlags)
{
	ApplyViewMode(VMI_Lit, true, ShowFlags);
	ShowFlags.SetMaterials(true);
	ShowFlags.SetLighting(true);
	ShowFlags.SetPostProcessing(true);
	// ToneMapper needs to be enabled, otherwise the screen will be very dark
	ShowFlags.SetTonemapper(true);
	// TemporalAA needs to be disabled, otherwise the previous frame might contaminate current frame.
	// Check: https://answers.unrealengine.com/questions/436060/low-quality-screenshot-after-setting-the-actor-pos.html for detail
	// Viewport->EngineShowFlags.SetTemporalAA(false);
	ShowFlags.SetTemporalAA(true);
}

void FViewMode::BufferVisualization(FEngineShowFlags& ShowFlags)
{
	ApplyViewMode(EViewModeIndex::VMI_VisualizeBuffer, true, ShowFlags); // This did more than just SetVisualizeBuffer(true);
	// EngineShowFlagOverride()

	// From ShowFlags.cpp
	ShowFlags.SetPostProcessing(true);
	// EngineShowFlags.SetMaterials(false);
	ShowFlags.SetMaterials(true);
	ShowFlags.SetVisualizeBuffer(true);


	//Viewport->EngineShowFlags.AmbientOcclusion = 0;
	//Viewport->EngineShowFlags.ScreenSpaceAO = 0;
	//Viewport->EngineShowFlags.Decals = 0;
	//Viewport->EngineShowFlags.DynamicShadows = 0;
	//Viewport->EngineShowFlags.GlobalIllumination = 0;
	//Viewport->EngineShowFlags.ScreenSpaceReflections = 0;

	// ToneMapper needs to be disabled
	ShowFlags.SetTonemapper(false);
	// TemporalAA needs to be disabled, or it will contaminate the following frame
	ShowFlags.SetTemporalAA(false);
}

void FViewMode::PostProcess(FEngineShowFlags& ShowFlags)
{
	ShowFlags.Rendering = true;
	ShowFlags.PostProcessing = true;
	// ShowFlags.PostProcessMaterial = true;
	ShowFlags.SetPostProcessMaterial(true);
	ShowFlags.StaticMeshes = true;

	/*
	// Set all flags to false, only enable useful flags.
	

	ApplyViewMode(EViewModeIndex::VMI_VisualizeBuffer, true, ShowFlags); // This did more than just SetVisualizeBuffer(true);
	// EngineShowFlagOverride()

	// From ShowFlags.cpp
	ShowFlags.SetPostProcessing(true);
	// EngineShowFlags.SetMaterials(false);
	ShowFlags.SetMaterials(true);
	ShowFlags.SetVisualizeBuffer(false);


	//Viewport->EngineShowFlags.AmbientOcclusion = 0;
	//Viewport->EngineShowFlags.ScreenSpaceAO = 0;
	//Viewport->EngineShowFlags.Decals = 0;
	//Viewport->EngineShowFlags.DynamicShadows = 0;
	//Viewport->EngineShowFlags.GlobalIllumination = 0;
	//Viewport->EngineShowFlags.ScreenSpaceReflections = 0;

	// ToneMapper needs to be disabled
	ShowFlags.SetTonemapper(false);
	// TemporalAA needs to be disabled, or it will contaminate the following frame
	ShowFlags.SetTemporalAA(false);
	*/
}

void FViewMode::VertexColor(FEngineShowFlags& ShowFlags)
{
	ApplyViewMode(VMI_Lit, true, ShowFlags);

	ShowFlags.SetMaterials(false);
	ShowFlags.SetLighting(false);
	ShowFlags.SetBSPTriangles(true);
	ShowFlags.SetVertexColors(true);
	ShowFlags.SetPostProcessing(false);
	ShowFlags.SetHMDDistortion(false);
	ShowFlags.SetTonemapper(false); // This won't take effect here

	GVertexColorViewMode = EVertexColorViewMode::Color;
}

void FViewMode::Unlit(FEngineShowFlags& ShowFlags)
{
	ApplyViewMode(VMI_Unlit, true, ShowFlags);
	ShowFlags.SetMaterials(false);
	ShowFlags.SetVertexColors(false);
	ShowFlags.SetLightFunctions(false);
	ShowFlags.SetLighting(false);
	ShowFlags.SetAtmosphericFog(false);
}
