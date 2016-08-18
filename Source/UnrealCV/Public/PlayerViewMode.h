// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommandDispatcher.h"
/**
 * Define different ViewModes of the scene
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
	void VertexColor();
	/** The ShowFlags of this mode will be set within debugger */
	void DebugMode();
	FExecStatus SetMode(const TArray<FString>& Args); // Check input arguments
	FExecStatus GetMode(const TArray<FString>& Args);
	void CreatePostProcessVolume();
	void ApplyPostProcess(FString ModeName);
private:
	void SetCurrentBufferVisualizationMode(FString ViewMode);
	FPlayerViewMode();
	FString CurrentViewMode;
	APostProcessVolume* PostProcessVolume = nullptr;
	void ClearPostProcess();
};
