// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommandDispatcher.h"
/**
 *
 */
class FViewMode
{
public:
	static FViewMode& Get();
	~FViewMode();
	void SetCurrentBufferVisualizationMode(FString ViewMode);
	void Depth();
	void Normal();
	void Lit();
	void Unlit();
	void Object();
	void BaseColor();
	FExecStatus SetMode(const TArray<FString>& Args); // Check input arguments
	FExecStatus GetMode(const TArray<FString>& Args);
	UWorld* World;
private:
	FViewMode();
	FString CurrentViewMode;
};
