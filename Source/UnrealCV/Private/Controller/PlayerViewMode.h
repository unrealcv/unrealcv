// Weichao Qiu @ 2016
#pragma once
#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/PostProcessVolume.h"
#include "Runtime/Engine/Public/ShowFlags.h"
#include "Runtime/Engine/Classes/Materials/Material.h"
#include "CommandDispatcher.h"
#include "PlayerViewMode.generated.h"

/**
 * Define different ViewModes of the scene
 * Provide functions for vset /viewmode
 */
UCLASS() 
class UNREALCV_API UPlayerViewMode : public UObject
{
	GENERATED_BODY()
public:
	// static UPlayerViewMode& Get();
	// ~UPlayerViewMode();
	void Depth();
	void DepthWorldUnits();
	void Normal();
	void Lit();
	void Unlit();
	void Object();
	void BaseColor();

	/** Set to VertexColor mode with painting objects */
	void VertexColor();

	/** Disable transparent materials */
	void NoTransparency();

	/** The ShowFlags of this mode will be set within debugger */
	void DebugMode();
	FExecStatus SetMode(const TArray<FString>& Args); // Check input arguments
	FExecStatus GetMode(const TArray<FString>& Args);
	APostProcessVolume* GetPostProcessVolume();
	void ApplyPostProcess(FString ModeName);

	/** Save the default rendering configuration of game for switching back from GT modes */
	void SaveGameDefault(FEngineShowFlags ShowFlags);

	UMaterial* GetMaterial(FString InModeName);
private:
	FEngineShowFlags* GameShowFlags;
	void SetCurrentBufferVisualizationMode(FString ViewMode);
	UPlayerViewMode();
	FString CurrentViewMode;
	void ClearPostProcess();

	TMap<FString, UMaterial*> PPMaterialMap;
};
