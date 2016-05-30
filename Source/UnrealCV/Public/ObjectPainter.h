#pragma once

/*
* Annotate objects in the scene with a unique color
*/
class UNREALCV_API FObjectPainter
{
private:
	ULevel* Level;
	FObjectPainter(ULevel* InLevel);

public:
	/** The assigned color for each object */
	TMap<FString, FColor> ObjectsColorMapping;

	/** The pointer for each object */
	TMap<FString, AActor*> ObjectsMapping;

	/** Vertex paint one object with Flood-Fill */
	bool PaintObject(AActor* Actor, const FColor& NewColor);

	/** Paint all objects */
	// TODO: Random color is not a good idea
	bool PaintRandomColors();

	/** Set the level for this painter */
	void SetLevel(ULevel* InLevel);

	/** Return the singleton of FObjectPainter */
	static FObjectPainter& Get();
};