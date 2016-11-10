#include "UnrealCVPrivate.h"
#include "PlayerViewMode.h"
#include "BufferVisualizationData.h"
#include "CommandDispatcher.h"
#include "Async.h"
#include "SceneViewport.h"
#include "ViewMode.h"
#include "GTCaptureComponent.h"
#include "ObjectPainter.h"

DECLARE_DELEGATE(ViewModeFunc)

FPlayerViewMode::FPlayerViewMode() : CurrentViewMode("lit") 
{
}

APostProcessVolume* FPlayerViewMode::GetPostProcessVolume()
{
	static APostProcessVolume* PostProcessVolume = nullptr;
	static UWorld* CurrentWorld = nullptr; // Check whether the world has been restarted.
	if (PostProcessVolume == nullptr || CurrentWorld != GWorld)
	{
		PostProcessVolume = GWorld->SpawnActor<APostProcessVolume>();
		PostProcessVolume->bUnbound = true;
		CurrentWorld = GWorld;
	}
	return PostProcessVolume;
}

FPlayerViewMode::~FPlayerViewMode() {}

FPlayerViewMode& FPlayerViewMode::Get()
{
	static FPlayerViewMode Singleton;
	return Singleton;
}

void FPlayerViewMode::SetCurrentBufferVisualizationMode(FString ViewMode)
{
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
		// AsyncTask(ENamedThreads::GameThread, [ViewMode]() {}); // & means capture by reference
		// The ICVar can only be set in GameThread, the CommandDispatcher already enforce this requirement.
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The BufferVisualization is not correctly configured."));
	}
}

void FPlayerViewMode::DepthWorldUnits()
{
	UGameViewportClient* Viewport = GWorld->GetGameViewport();
	FViewMode::BufferVisualization(Viewport->EngineShowFlags);
	SetCurrentBufferVisualizationMode(TEXT("SceneDepthWorldUnits"));
}

void FPlayerViewMode::Depth()
{
	this->ApplyPostProcess("vis_depth");
}

void FPlayerViewMode::Normal()
{
	this->ApplyPostProcess("normal");
}

void FPlayerViewMode::BaseColor()
{
	SetCurrentBufferVisualizationMode(TEXT("BaseColor"));
}

void FPlayerViewMode::Lit()
{
	this->ClearPostProcess();
	if (GameShowFlags == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("The lit mode is not correctly configured."));
		return;
	}
	GWorld->GetGameViewport()->EngineShowFlags = *GameShowFlags;
}

void FPlayerViewMode::Unlit()
{
	auto Viewport = GWorld->GetGameViewport();
	FViewMode::Unlit(Viewport->EngineShowFlags);
}

void FPlayerViewMode::ClearPostProcess()
{
	GetPostProcessVolume()->BlendWeight = 0;
}

void FPlayerViewMode::ApplyPostProcess(FString ModeName)
{
	UGameViewportClient* GameViewportClient = GWorld->GetGameViewport();
	FSceneViewport* SceneViewport = GameViewportClient->GetGameViewport();

	FViewMode::PostProcess(GameViewportClient->EngineShowFlags);

	UMaterial* Material = UGTCaptureComponent::GetMaterial(ModeName);
	APostProcessVolume* PostProcessVolume = GetPostProcessVolume();
	PostProcessVolume->Settings.WeightedBlendables.Array.Empty();

	// PostProcessVolume->AddOrUpdateBlendable(Material);
	PostProcessVolume->Settings.AddBlendable(Material, 1);
	PostProcessVolume->BlendWeight = 1;
}

void FPlayerViewMode::DebugMode()
{
	ApplyPostProcess("debug");
}

// TODO: Clean up this messy function.
void PaintObjects()
{
	APlayerController* PlayerController = GWorld->GetFirstPlayerController();
	check(PlayerController);
	APawn* Pawn = PlayerController->GetPawn();
	check(Pawn);
	FObjectPainter::Get().Reset(Pawn->GetLevel());
	FObjectPainter::Get().PaintColors();
}

void FPlayerViewMode::Object()
{
	PaintObjects();
	auto Viewport = GWorld->GetGameViewport();
	FViewMode::VertexColor(Viewport->EngineShowFlags);
	// ApplyPostProcess("object_mask");
}

FExecStatus FPlayerViewMode::SetMode(const TArray<FString>& Args) // Check input arguments
{
	UE_LOG(LogUnrealCV, Warning, TEXT("Run SetMode %s"), *Args[0]);

	static TMap<FString, ViewModeFunc>* ViewModeHandlers;

	if (ViewModeHandlers == nullptr)
	{
		ViewModeHandlers = new TMap<FString, ViewModeFunc>();
		ViewModeHandlers->Add(TEXT("depth"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::Depth));
		ViewModeHandlers->Add(TEXT("normal"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::Normal));
		ViewModeHandlers->Add(TEXT("object_mask"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::Object));
		// ViewModeHandlers->Add(TEXT("vertex_color"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::VertexColor));
		ViewModeHandlers->Add(TEXT("lit"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::Lit));
		ViewModeHandlers->Add(TEXT("unlit"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::Unlit));
		ViewModeHandlers->Add(TEXT("base_color"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::BaseColor));
		ViewModeHandlers->Add(TEXT("debug"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::DebugMode));
		ViewModeHandlers->Add(TEXT("wireframe"), ViewModeFunc::CreateLambda([]() { FViewMode::Wireframe(GWorld->GetGameViewport()->EngineShowFlags);  }));
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
			UE_LOG(LogUnrealCV, Warning, TEXT("Unrecognized ViewMode %s"), *ViewMode);
			bSuccess = false;
			return FExecStatus::Error(FString::Printf(TEXT("Can not set ViewMode to %s"), *ViewMode));
		}
	}
	// Send successful message back
	// The network manager should be blocked and wait for response. So the client needs to be universally accessible?
	return FExecStatus::Error(TEXT("Arguments to SetMode are incorrect"));
}


FExecStatus FPlayerViewMode::GetMode(const TArray<FString>& Args) // Check input arguments
{
	return FExecStatus::OK(CurrentViewMode);
}

void FPlayerViewMode::SaveGameDefault(FEngineShowFlags ShowFlags)
{
	if (this->GameShowFlags != nullptr)
	{
		delete this->GameShowFlags;
		this->GameShowFlags = nullptr;
	}
	GameShowFlags = new FEngineShowFlags(ShowFlags);
}