#pragma once

#include "ExecStatus.h"
/*
* Annotate objects in the scene with a unique color
*/
class UNREALCV_API FObjectPainter
{
private:
	ULevel* Level;
	FObjectPainter(ULevel* InLevel);
	/** The assigned color for each object */
	TMap<FString, FColor> ObjectColorMap;

public:
	/** A list of paintable objects in a level */
	TMap<FString, AActor*>& GetObjectMap(ULevel* Level);

	/** Vertex paint one object with Flood-Fill */
	bool PaintObject(AActor* Actor, const FColor& Color, bool IsColorGammaEncoded = true);

	/** Paint all objects */
	bool PaintColors();

	/** Set the level for this painter */
	void SetLevel(ULevel* InLevel);

	/** Return the singleton of FObjectPainter */
	static FObjectPainter& Get();

	/** Return a list of actors in the level */
	FExecStatus GetObjectList();

	/** Get the object color */
	FExecStatus GetActorColor(FString ObjectName);

	/** Functions to support CommandDispatcher */
	FExecStatus SetActorColor(FString ObjectName, FColor Color);

	/** Get a pointor to an object */
	AActor* GetObject(FString ObjectName);
};
