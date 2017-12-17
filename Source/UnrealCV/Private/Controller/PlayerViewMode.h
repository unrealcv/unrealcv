// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommandDispatcher.h"
/**
 * Define different ViewModes of the scene
 * Provide functions for vset /viewmode
 */
class UNREALCV_API FPlayerViewMode
{
public:
	static FPlayerViewMode& Get();
	~FPlayerViewMode();
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
private:
	FEngineShowFlags* GameShowFlags;
	void SetCurrentBufferVisualizationMode(FString ViewMode);
	FPlayerViewMode();
	FString CurrentViewMode;
	void ClearPostProcess();
};
