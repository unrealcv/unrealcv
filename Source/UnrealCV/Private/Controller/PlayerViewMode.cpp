#include "PlayerViewMode.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "Materials/Material.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/Engine/Public/BufferVisualizationData.h"

#include "CommandDispatcher.h"
#include "ViewMode.h"
#include "UnrealcvServer.h"
#include "UnrealcvLog.h"

DECLARE_DELEGATE(ViewModeFunc)

TMap<FString, UMaterial*> UPlayerViewMode::PPMaterialMap;
FEngineShowFlags* UPlayerViewMode::GameShowFlags = nullptr;

UPlayerViewMode::UPlayerViewMode() : CurrentViewMode("lit")
{
	// Load material for visualization
	TMap<FString, FString> MaterialPathMap;
	MaterialPathMap.Add(TEXT("depth"), TEXT("Material'/UnrealCV/SceneDepthWorldUnits.SceneDepthWorldUnits'"));
	MaterialPathMap.Add(TEXT("plane_depth"), TEXT("Material'/UnrealCV/ScenePlaneDepthWorldUnits.ScenePlaneDepthWorldUnits'"));
	MaterialPathMap.Add(TEXT("vis_depth"), TEXT("Material'/UnrealCV/SceneDepth.SceneDepth'"));
	MaterialPathMap.Add(TEXT("debug"), TEXT("Material'/UnrealCV/debug.debug'"));
	MaterialPathMap.Add(TEXT("object_mask"), TEXT("Material'/UnrealCV/VertexColorMaterial.VertexColorMaterial'"));
	MaterialPathMap.Add(TEXT("normal"), TEXT("Material'/UnrealCV/WorldNormal.WorldNormal'"));
	MaterialPathMap.Add(TEXT("optical_flow"), TEXT("Material'/UnrealCV/OpticalFlowMaterial.OpticalFlowMaterial'"));
	FString OpaqueMaterialName = "Material'/UnrealCV/OpaqueMaterial.OpaqueMaterial'";
	MaterialPathMap.Add(TEXT("opaque"), OpaqueMaterialName);

	PPMaterialMap = {};
	for (auto& Elem : MaterialPathMap)
	{
		FString ModeName = Elem.Key;
		FString MaterialPath = Elem.Value;
		// ConstructorHelpers::FObjectFinder<UMaterial> Material(*MaterialPath); 
		// // ConsturctorHelpers is only available in the CTOR of UObject.
		// UMaterial* MaterialObject = Cast<UMaterial>(Material.Object);
		UMaterial* MaterialObject = Cast<UMaterial>(StaticLoadObject(
			UMaterial::StaticClass(),
			nullptr,
			*MaterialPath
		));

		if (MaterialObject != nullptr)
		{
			PPMaterialMap.Add(ModeName, MaterialObject);
		}
		else
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Failed to load material for mode: %s from path: %s"), *ModeName, *MaterialPath);
		}
	}

}

UMaterial* UPlayerViewMode::GetMaterial(FString InModeName)
{
	if (!PPMaterialMap.Contains(InModeName))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Can not recognize visualization mode %s"), *InModeName);
		return nullptr;
	}
	UMaterial* Material = PPMaterialMap.FindRef(InModeName);
	if (Material == nullptr)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not recognize visualization mode %s"), *InModeName);
	}
	return Material;
}

APostProcessVolume* UPlayerViewMode::GetPostProcessVolume()
{
	UWorld* World = FUnrealcvServer::Get().GetWorld();
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

// UPlayerViewMode::~UPlayerViewMode() {}

// UPlayerViewMode& UPlayerViewMode::Get()
// {
// 	static UPlayerViewMode* Singleton = NewObject<UPlayerViewMode>();
// 	return *Singleton;
// }

void UPlayerViewMode::SetCurrentBufferVisualizationMode(FString ViewMode)
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

void UPlayerViewMode::DepthWorldUnits()
{
	UWorld* World = FUnrealcvServer::Get().GetWorld();
	UGameViewportClient* Viewport = World->GetGameViewport();
	Viewport->SetViewMode(VMI_Lit);
	FViewMode::BufferVisualization(Viewport->EngineShowFlags);
	SetCurrentBufferVisualizationMode(TEXT("SceneDepthWorldUnits"));
}

void UPlayerViewMode::Depth()
{
	FUnrealcvServer::Get().GetWorld()->GetGameViewport()->SetViewMode(VMI_Lit);
	this->ApplyPostProcess("vis_depth");
}

void UPlayerViewMode::Normal()
{
	FUnrealcvServer::Get().GetWorld()->GetGameViewport()->SetViewMode(VMI_Lit);
	this->ApplyPostProcess("normal");
}

void UPlayerViewMode::OpticalFlow()
{
	FUnrealcvServer::Get().GetWorld()->GetGameViewport()->SetViewMode(VMI_Lit);
	this->ApplyPostProcess("optical_flow");
}

void UPlayerViewMode::BaseColor()
{
	FUnrealcvServer::Get().GetWorld()->GetGameViewport()->SetViewMode(VMI_Lit);
	SetCurrentBufferVisualizationMode(TEXT("BaseColor"));
}

void UPlayerViewMode::Lit()
{
	UWorld* World = FUnrealcvServer::Get().GetWorld();
	this->ClearPostProcess();
	if (GameShowFlags == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("The default show flags is not correctly configured."));
		return;
	}
	auto Viewport = World->GetGameViewport();
	Viewport->SetViewMode(VMI_Lit);
	Viewport->EngineShowFlags = *GameShowFlags;
	// FViewMode::Lit(Viewport->EngineShowFlags);
}

void UPlayerViewMode::Unlit()
{
	UWorld* World = FUnrealcvServer::Get().GetWorld();
	this->ClearPostProcess();
	if (GameShowFlags == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("The default show flags is not correctly configured."));
		return;
	}
	auto Viewport = World->GetGameViewport();
	Viewport->SetViewMode(VMI_Unlit);
	Viewport->EngineShowFlags = *GameShowFlags;
}

void UPlayerViewMode::DebugMode()
{
	ApplyPostProcess("debug");
}

void UPlayerViewMode::Object()
{
	UWorld* World = FUnrealcvServer::Get().GetWorld();
	auto Viewport = World->GetGameViewport();
	Viewport->SetViewMode(VMI_Lit);
	FViewMode::VertexColor(Viewport->EngineShowFlags);
	this->ClearPostProcess();
	// ApplyPostProcess("object_mask");
}

void UPlayerViewMode::ClearPostProcess()
{
	GetPostProcessVolume()->BlendWeight = 0;
}

void UPlayerViewMode::ApplyPostProcess(FString ModeName)
{
	UWorld* World = FUnrealcvServer::Get().GetWorld();
	UGameViewportClient* GameViewportClient = World->GetGameViewport();
	// FSceneViewport* SceneViewport = GameViewportClient->GetGameViewport();

	FViewMode::PostProcess(GameViewportClient->EngineShowFlags);

	UMaterial* Material = GetMaterial(ModeName);
	if (!IsValid(Material))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Can not find material for mode %s"), *ModeName);
		return;
	}

	APostProcessVolume* PostProcessVolume = GetPostProcessVolume();
	// PostProcessVolume->bEnabled = true;
	PostProcessVolume->Settings.WeightedBlendables.Array.Empty();
	// PostProcessVolume->AddOrUpdateBlendable(Material);
	PostProcessVolume->Settings.AddBlendable(Material, 1);
	PostProcessVolume->BlendWeight = 1;
}



FExecStatus UPlayerViewMode::SetMode(const TArray<FString>& Args) // Check input arguments
{
	UWorld* World = FUnrealcvServer::Get().GetWorld();
	UE_LOG(LogUnrealCV, Warning, TEXT("Run SetMode %s"), *Args[0]);

	static TMap<FString, ViewModeFunc>* ViewModeHandlers;

	if (ViewModeHandlers == nullptr)
	{
		ViewModeHandlers = new TMap<FString, ViewModeFunc>();
		ViewModeHandlers->Add(TEXT("depth"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::Depth));
		ViewModeHandlers->Add(TEXT("normal"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::Normal));
		ViewModeHandlers->Add(TEXT("optical_flow"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::OpticalFlow));
		ViewModeHandlers->Add(TEXT("object_mask"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::Object));
		// ViewModeHandlers->Add(TEXT("vertex_color"), ViewModeFunc::CreateRaw(this, &FPlayerViewMode::VertexColor));
		ViewModeHandlers->Add(TEXT("lit"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::Lit));
		ViewModeHandlers->Add(TEXT("unlit"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::Unlit));
		ViewModeHandlers->Add(TEXT("base_color"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::BaseColor));
		ViewModeHandlers->Add(TEXT("debug"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::DebugMode));
		ViewModeHandlers->Add(TEXT("wireframe"), ViewModeFunc::CreateLambda([World]() { FViewMode::Wireframe(World->GetGameViewport()->EngineShowFlags);  }));
		ViewModeHandlers->Add(TEXT("vertex_color"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::VertexColor));
		ViewModeHandlers->Add(TEXT("no_transparency"), ViewModeFunc::CreateUObject(this, &UPlayerViewMode::NoTransparency));
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


FExecStatus UPlayerViewMode::GetMode(const TArray<FString>& Args) // Check input arguments
{
	return FExecStatus::OK(CurrentViewMode);
}

void UPlayerViewMode::SaveGameDefault(FEngineShowFlags ShowFlags)
{
	if (GameShowFlags != nullptr)
	{
		delete GameShowFlags;
		GameShowFlags = nullptr;
	}
	GameShowFlags = new FEngineShowFlags(ShowFlags);
}

void UPlayerViewMode::VertexColor()
{
	auto Viewport = FUnrealcvServer::Get().GetWorld()->GetGameViewport();
	Viewport->SetViewMode(VMI_Lit);
	FViewMode::VertexColor(Viewport->EngineShowFlags);
}

void UPlayerViewMode::NoTransparency()
{
	// Iterate over all the materials in the scene and replace transparent materials to non-transparent 
	for (TActorIterator<AActor> ActorItr(FUnrealcvServer::Get().GetWorld()); ActorItr; ++ActorItr)
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
							UMaterial* OpaqueMaterial = GetMaterial("opaque");
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
