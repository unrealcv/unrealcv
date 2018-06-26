// Weichao Qiu @ 2017
#pragma once

#include "Runtime/Engine/Classes/GameFramework/Actor.h"

class FVertexColorPainter
{
public:
	/** Paint the actor with color */
	static void PaintVertexColor(AActor* Actor, const FColor& PaintColor);
};
