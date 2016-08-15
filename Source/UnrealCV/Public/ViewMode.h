#pragma once
#include "Engine.h"

/**
 * A helper class to set EngineShowFlags
 */
class UNREALCV_API FViewMode
{
public:
	static void Lit(FEngineShowFlags& ShowFlags);

	static void BufferVisualization(FEngineShowFlags& ShowFlags);

	static void VertexColor(FEngineShowFlags& ShowFlags);

	static void Unlit(FEngineShowFlags& ShowFlags);

	static void PostProcess(FEngineShowFlags& ShowFlags);

	static void Wireframe(FEngineShowFlags& ShowFlags);
};
