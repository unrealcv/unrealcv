// Fill out your copyright notice in the Description page of Project Settings.

// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "CameraViewMode.h"
#include "BufferVisualizationData.h"
#include "CommandDispatcher.h"
#include "Async.h"
#include "SceneViewport.h"
#include "ViewMode.h"
#include "GTCapturer.h"

#define REG_VIEW(COMMAND, DESC, DELEGATE) \
		IConsoleManager::Get().RegisterConsoleCommand(TEXT(COMMAND), TEXT(DESC), \
			FConsoleCommandWithWorldDelegate::CreateStatic(DELEGATE), ECVF_Default);

DECLARE_DELEGATE(ViewModeFunc)

FCameraViewMode::FCameraViewMode() : World(NULL), CurrentViewMode("lit") {}

FCameraViewMode::~FCameraViewMode() {}

FCameraViewMode& FCameraViewMode::Get()
{
	static FCameraViewMode Singleton;
	return Singleton;
}



void FCameraViewMode::SetCurrentBufferVisualizationMode(FString ViewMode)
{
	check(World);

	// A complete list can be found from Engine/Config/BaseEngine.ini, Engine.BufferVisualizationMaterials
	// TODO: BaseColor is weird, check BUG.
	// No matter in which thread, ICVar needs to be set in the GameThread

	// AsyncTask is from https://answers.unrealengine.com/questions/317218/execute-code-on-gamethread.html
	// lambda is from http://stackoverflow.com/questions/26903602/an-enclosing-function-local-variable-cannot-be-referenced-in-a-lambda-body-unles
	// Make this async is risky, TODO:
	static IConsoleVariable* ICVar = IConsoleManager::Get().FindConsoleVariable(FBufferVisualizationData::GetVisualizationTargetConsoleCommandName());
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

void FCameraViewMode::DepthWorldUnits()
{
	UGameViewportClient* Viewport = GWorld->GetGameViewport();
	FViewMode::BufferVisualization(Viewport->EngineShowFlags);
	SetCurrentBufferVisualizationMode(TEXT("SceneDepthWorldUnits"));
}

void FCameraViewMode::Depth()
{
	UGameViewportClient* Viewport = GWorld->GetGameViewport();
	FViewMode::BufferVisualization(Viewport->EngineShowFlags);
	SetCurrentBufferVisualizationMode(TEXT("SceneDepth"));
}

void FCameraViewMode::Normal()
{
	UGameViewportClient* Viewport = GWorld->GetGameViewport();
	FViewMode::BufferVisualization(Viewport->EngineShowFlags);
	SetCurrentBufferVisualizationMode(TEXT("WorldNormal"));
}

void FCameraViewMode::BaseColor()
{
	SetCurrentBufferVisualizationMode(TEXT("BaseColor"));
}

void FCameraViewMode::Lit()
{
	check(World);
	auto Viewport = World->GetGameViewport();
	FViewMode::Lit(Viewport->EngineShowFlags);
}

void FCameraViewMode::Unlit()
{
	check(World);
	auto Viewport = World->GetGameViewport();
	FViewMode::Unlit(Viewport->EngineShowFlags);
}

void FCameraViewMode::DebugMode()
{
	// float DisplayGamma = FRenderTarget::GetDisplayGamma();
	// GetDisplayGamma();
	UGameViewportClient* GameViewportClient = World->GetGameViewport();
	FSceneViewport* SceneViewport = GameViewportClient->GetGameViewport();
	float DisplayGamma = SceneViewport->GetDisplayGamma();


	static APostProcessVolume* PostProcessVolume;
	if (PostProcessVolume == nullptr)
	{
		PostProcessVolume = GWorld->SpawnActor<APostProcessVolume>();
		// PostProcessVolume = NewObject<APostProcessVolume>();
		// PostProcessVolume->AddToRoot();
		PostProcessVolume->bUnbound = true;
		// PostProcessVolume->RegisterAllComponents();
	}
	UMaterial* Material = UGTCapturer::GetMaterial("debug");
	PostProcessVolume->AddOrUpdateBlendable(Material);
}

void FCameraViewMode::Object()
{
	check(World);

	auto Viewport = World->GetGameViewport();
	FViewMode::VertexColor(Viewport->EngineShowFlags);
}

FExecStatus FCameraViewMode::SetMode(const TArray<FString>& Args) // Check input arguments
{
	UE_LOG(LogTemp, Warning, TEXT("Run SetMode %s"), *Args[0]);

	static TMap<FString, ViewModeFunc>* ViewModeHandlers;

	if (ViewModeHandlers == nullptr)
	{
		ViewModeHandlers = new TMap<FString, ViewModeFunc>();
		ViewModeHandlers->Add(TEXT("depth"), ViewModeFunc::CreateRaw(this, &FCameraViewMode::Depth));
		ViewModeHandlers->Add(TEXT("normal"), ViewModeFunc::CreateRaw(this, &FCameraViewMode::Normal));
		ViewModeHandlers->Add(TEXT("object_mask"), ViewModeFunc::CreateRaw(this, &FCameraViewMode::Object));
		ViewModeHandlers->Add(TEXT("lit"), ViewModeFunc::CreateRaw(this, &FCameraViewMode::Lit));
		ViewModeHandlers->Add(TEXT("unlit"), ViewModeFunc::CreateRaw(this, &FCameraViewMode::Unlit));
		ViewModeHandlers->Add(TEXT("base_color"), ViewModeFunc::CreateRaw(this, &FCameraViewMode::BaseColor));
		ViewModeHandlers->Add(TEXT("debug"), ViewModeFunc::CreateRaw(this, &FCameraViewMode::DebugMode));
	}

	// Check args
	if (Args.Num() == 1)
	{

		FString ViewMode = Args[0].ToLower();
		bool bSuccess = true;

		ViewModeFunc* Func = ViewModeHandlers->Find(ViewMode);
		if (Func)
		{
			Func->Execute();
			CurrentViewMode = ViewMode;
			return FExecStatus::OK();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unrecognized ViewMode %s"), *ViewMode);
			bSuccess = false;
			return FExecStatus::Error(FString::Printf(TEXT("Can not set ViewMode to %s"), *ViewMode));
		}
	}
	// Send successful message back
	// The network manager should be blocked and wait for response. So the client needs to be universally accessible?
	return FExecStatus::Error(TEXT("Arguments to SetMode are incorrect"));
}


FExecStatus FCameraViewMode::GetMode(const TArray<FString>& Args) // Check input arguments
{
	UE_LOG(LogTemp, Warning, TEXT("Run GetMode, The mode is %s"), *CurrentViewMode);
	return FExecStatus::OK(CurrentViewMode);
}
