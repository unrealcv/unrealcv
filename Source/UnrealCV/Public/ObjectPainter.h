#pragma once

class UNREALCV_API FObjectPainter
{
private:
	TMap<FString, FColor> ObjectsColorMapping;
	TMap<FString, AActor*> ObjectsMapping;
	ULevel* Level;

public:
	FObjectPainter(ULevel* InLevel);
	// Vertex One Object with Flood-Fill
	bool PaintObject(AActor* Actor, const FColor& NewColor);
	// Paint all objects
	bool PaintRandomColors();
	// FObjectPainter& Get();
};