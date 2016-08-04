#pragma once
#include "Engine.h"

class UNREALCV_API FViewMode
{
public:
	FViewMode(FEngineShowFlags& InShowFlags) : ShowFlags(InShowFlags)
	{
	}

	void VisDepth();

	void Depth();

	/*
	void Normal();

	void Lit();
	*/
private:
	FEngineShowFlags& ShowFlags;

	void SetCurrentBufferVisualizationMode(FString ViewMode);
};
