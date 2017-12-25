// Weichao Qiu @ 2017
#pragma once

/** Annotate each object instance with a unique color */
class FObjectAnnotator
{
public:
	FObjectAnnotator();

	/** Annotate all StaticMesh actor in the world */
	void AnnotateStaticMesh();

	bool SetAnnotationColor(AActor* Actor, const FColor& AnnotationColor);
	bool GetAnnotationColor(AActor* Actor, FColor& AnnotationColor);

	bool SetVertexColor(AActor* Actor, const FColor& AnnotationColor);
	bool GetVertexColor(AActor* Actor, FColor& AnnotationColor);
	// void SetObjectStencilId(FString ObjectId, const uint8 StencilId);
	// void GetObjectStencilId(FString ObjectId, uint8& StencilId);
	void SetObjectStencilId(AActor* Actor, const uint8 StencilId);
	void GetObjectStencilId(AActor* Actor, uint8& StencilId);

	/** Dump the annotation for each object */
	void SaveAnnotation(FString JsonFilename);
	/** Load the annotation for each object */
	void LoadAnnotation(FString JsonFilename);

	/** Assign a unique new color for this object */
	FColor GetDefaultColor(AActor* Actor);

private:
	TMap<FString, FColor> AnnotationColors;
};
