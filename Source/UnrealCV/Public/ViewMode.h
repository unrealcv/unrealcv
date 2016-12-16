#pragma once
#include "Engine.h"

/**
 * A helper class to set EngineShowFlags
 */
class UNREALCV_API FViewMode
{
public:
	/** Deprecated: Set game to normal rendering */
	// static void Lit(FEngineShowFlags& ShowFlags);

	/** Deprecated: Set game to buffer visualization mode */
	static void BufferVisualization(FEngineShowFlags& ShowFlags);

	/** ViewMode for object instance mask */
	static void VertexColor(FEngineShowFlags& ShowFlags);

	/** ViewMode for ground truth types implemented with PostProcess material */
	static void PostProcess(FEngineShowFlags& ShowFlags);

	static void Unlit(FEngineShowFlags& ShowFlags);

	/** Wireframe viewmode for debug */
	static void Wireframe(FEngineShowFlags& ShowFlags);

	/** Use the source flags to set the object visibility of target */
	static void SetVisibility(FEngineShowFlags& Target, FEngineShowFlags& Source);
};
