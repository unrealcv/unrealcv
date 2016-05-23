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
	TMap<FString, FColor> ObjectsColorMapping;
	TMap<FString, AActor*> ObjectsMapping;
	// Vertex One Object with Flood-Fill
	bool PaintObject(AActor* Actor, const FColor& NewColor);
	// Paint all objects
	bool PaintRandomColors();
	void SetLevel(ULevel* InLevel);
	static FObjectPainter& Get();
};