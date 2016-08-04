#include "UnrealCVPrivate.h"
#include "ViewMode.h"
#include "BufferVisualizationData.h"

void FViewMode::Depth()
{
	SetCurrentBufferVisualizationMode(TEXT("SceneDepthWorldUnits"));
}

void FViewMode::VisDepth()
{
	SetCurrentBufferVisualizationMode(TEXT("SceneDepth"));
}

void FViewMode::SetCurrentBufferVisualizationMode(FString ViewMode)
{
	ApplyViewMode(EViewModeIndex::VMI_VisualizeBuffer, true, ShowFlags); // This did more than just SetVisualizeBuffer(true);

	// From ShowFlags.cpp
	ShowFlags.SetPostProcessing(true);
	// Viewport->EngineShowFlags.SetMaterials(false);
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

	// A complete list can be found from Engine/Config/BaseEngine.ini, Engine.BufferVisualizationMaterials
	// TODO: BaseColor is weird, check BUG.
	// No matter in which thread, ICVar needs to be set in the GameThread

	// AsyncTask is from https://answers.unrealengine.com/questions/317218/execute-code-on-gamethread.html
	// lambda is from http://stackoverflow.com/questions/26903602/an-enclosing-function-local-variable-cannot-be-referenced-in-a-lambda-body-unles
	// Make this async is risky, TODO:
	FString ICVarName = FBufferVisualizationData::GetVisualizationTargetConsoleCommandName();
	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	static IConsoleVariable* ICVar = ConsoleManager.FindConsoleVariable(*ICVarName);
	if (ICVar)
	{
		ICVar->Set(*ViewMode, ECVF_SetByCode); // TODO: Should wait here for a moment.
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("The BufferVisualization is not correctly configured."));
	}
	// AsyncTask(ENamedThreads::GameThread, [ViewMode]() {}); // & means capture by reference
	// The ICVar can only be set in GameThread, the CommandDispatcher already enforce this requirement.
}
