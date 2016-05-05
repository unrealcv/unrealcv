// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "RealisticRendering.h"
/**
 * 
 */
class FViewMode
{
public:
	FViewMode();
	~FViewMode();
	static void Depth(UWorld *World);
	static void SetCurrentBufferVisualizationMode(FString ViewMode);
	static void Normal(UWorld *World);
	static void Lit(UWorld* World);
	static void Unlit(UWorld* World);
	static void Object(UWorld* World);
	static void RegisterCommands();
	static void SetMode(const TArray<FString>& Args); // Check input arguments
	static void GetMode(const TArray<FString>& Args);
};
