#pragma once

#include "ExecStatus.h"
/*
* Annotate objects in the scene with a unique color
* Used to paint vertex color
*/
class UNREALCV_API FObjectPainter
{
private:
	/** The level this ObjectPainter associated with */
	ULevel* Level;

	FObjectPainter() {}
	/** The assigned color for each object */
	TMap<FString, FColor> Id2Color;
	/** A list of paintable objects */
	TMap<FString, AActor*> Id2Actor;

public:
	/** Return the singleton of FObjectPainter */
	static FObjectPainter& Get();

	/** Reset this to uninitialized state */
	void Reset(ULevel* InLevel);

	/** Vertex paint one object with Flood-Fill */
	bool PaintObject(AActor* Actor, const FColor& Color, bool IsColorGammaEncoded = true);

	/** Get a pointor to an object */
	AActor* GetObject(FString ObjectName);

	/** Return a list of actors in the level */
	FExecStatus GetObjectList();

	/** Get the object color */
	FExecStatus GetActorColor(FString ActorId);

	/** Functions to support CommandDispatcher */
	FExecStatus SetActorColor(FString ActorId, FColor Color);
};
