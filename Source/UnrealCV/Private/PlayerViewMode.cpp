#include "UnrealCVPrivate.h"
#include "PlayerViewMode.h"
#include "BufferVisualizationData.h"
#include "CommandDispatcher.h"
#include "Async/Async.h"
#include "Slate/SceneViewport.h"
#include "ViewMode.h"
#include "GTCaptureComponent.h"
#include "ObjectPainter.h"
#include "CaptureManager.h"

DECLARE_DELEGATE(ViewModeFunc)

FPlayerViewMode::FPlayerViewMode() : CurrentViewMode("lit")
{
}

APostProcessVolume* FPlayerViewMode::GetPostProcessVolume()
{
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
	static APostProcessVolume* PostProcessVolume = nullptr;
	static UWorld* CurrentWorld = nullptr; // Check whether the world has been restarted.
	if (PostProcessVolume == nullptr || CurrentWorld != World)
	{
		PostProcessVolume = World->SpawnActor<APostProcessVolume>();
		PostProcessVolume->bUnbound = true;
		CurrentWorld = World;
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
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
	UGameViewportClient* Viewport = World->GetGameViewport();
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
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
	this->ClearPostProcess();
	if (GameShowFlags == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("The lit mode is not correctly configured."));
		return;
	}
	World->GetGameViewport()->EngineShowFlags = *GameShowFlags;
	// FViewMode::Lit(Viewport->EngineShowFlags);
}

void FPlayerViewMode::Unlit()
{
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
	auto Viewport = World->GetGameViewport();
	FViewMode::Unlit(Viewport->EngineShowFlags);
}

void FPlayerViewMode::ClearPostProcess()
{
	GetPostProcessVolume()->BlendWeight = 0;
}

void FPlayerViewMode::ApplyPostProcess(FString ModeName)
{
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
	UGameViewportClient* GameViewportClient = World->GetGameViewport();
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

void FPlayerViewMode::Object()
{
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
	auto Viewport = World->GetGameViewport();
	FViewMode::VertexColor(Viewport->EngineShowFlags);
	// ApplyPostProcess("object_mask");
}

FExecStatus FPlayerViewMode::SetMode(const TArray<FString>& Args) // Check input arguments
{
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
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
		ViewModeHandlers->Add(TEXT("wireframe"), ViewModeFunc::CreateLambda([World]() { FViewMode::Wireframe(World->GetGameViewport()->EngineShowFlags);  }));
		ViewModeHandlers->Add(TEXT("vertex_color"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::VertexColor));
		ViewModeHandlers->Add(TEXT("no_transparency"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::NoTransparency));
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

void FPlayerViewMode::VertexColor()
{
	auto Viewport = FUE4CVServer::Get().GetGameWorld()->GetGameViewport();
	FViewMode::VertexColor(Viewport->EngineShowFlags);
}

void FPlayerViewMode::NoTransparency()
{
	// Iterate over all the materials in the scene and replace transparent materials to non-transparent 
	for (TActorIterator<AActor> ActorItr(FUE4CVServer::Get().GetGameWorld()); ActorItr; ++ActorItr)
	{
		// Iterate over all the material of the actor
		TArray<UActorComponent*> TheComponents;
		(*ActorItr)->GetComponents(TheComponents);

		for (auto ComponentIt = TheComponents.CreateIterator(); ComponentIt; ++ComponentIt)
		{
			UStaticMeshComponent *OneComponent = Cast<UStaticMeshComponent>(*ComponentIt);
			if (OneComponent != NULL)
			{
				for (int ComponentMaterialIdx = 0; ComponentMaterialIdx < OneComponent->GetNumMaterials(); ++ComponentMaterialIdx)
				{
					UMaterialInterface* MaterialInterface = OneComponent->GetMaterial(ComponentMaterialIdx);
					if (MaterialInterface != nullptr)
					{
						EBlendMode BlendMode = MaterialInterface->GetBlendMode();
						// OneComponent->SetMaterial(ComponentMaterialIdx, DynamicMaterialInstance);
						if (BlendMode == EBlendMode::BLEND_Translucent)
						{
							UMaterial* OpaqueMaterial = FCaptureManager::Get().GetCamera(0)->GetMaterial("opaque");
							OneComponent->SetMaterial(ComponentMaterialIdx, OpaqueMaterial);

							// Change this material to an opaque material.
							FString MaterialName = MaterialInterface->GetFullName();
							UE_LOG(LogUnrealCV, Warning, TEXT("%s is transparent"), *MaterialName);
						}
					}
				}
			}
		}
	}
}
