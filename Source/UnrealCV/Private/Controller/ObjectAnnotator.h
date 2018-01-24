// Weichao Qiu @ 2017
#pragma once

// Generate a color for annotating an object
class FColorGenerator
{

};

/** FObjectAnnotator supports annotate and update the annotation of an object,
it also keeps tracking of the annotation of each actor
Annotate each object instance with a unique color, three ways to annotate an object
1. VertexColor, which is used in unrealcv before v0.4, but not supported by UE4.17+
2. CustomDepthStencil, which is used in AirSim, but only supports 0 - 255 and needs to modify project setting
3. AnnotationComponent, which generates a dummy annotation component on the fly, which is used after unrealcv v0.4
*/
class FObjectAnnotator
{
public:
	FObjectAnnotator();

	// Annotate all StaticMesh actor in the world
	void AnnotateWorld(UWorld* World);

	// Annotate actor
	void SetAnnotationColor(AActor* Actor, const FColor& AnnotationColor);

	// Get annotation color for an actor
	void GetAnnotationColor(AActor* Actor, FColor& AnnotationColor);

private:
	// Get all annotable actors in the world
	void GetAnnotableActors(UWorld* World, TArray<AActor*>& ActorArray);

	// Annotate with VertexColor
	void PaintVertexColor(AActor* Actor, const FColor& AnnotationColor);
	// Annotate with AnnotationComponent
	void CreateAnnotationComponent(AActor* Actor, const FColor& AnnotationColor);
	// Update existing annotation components with a new color
	void UpdateAnnotationComponent(AActor* Actor, const FColor& AnnotationColor);

	// Use AActor* not FString ActorId
	// void SetObjectStencilId(FString ObjectId, const uint8 StencilId);
	// void GetObjectStencilId(FString ObjectId, uint8& StencilId);
	// void SetObjectStencilId(AActor* Actor, const uint8 StencilId);

	/** Assign a unique new color for this object */
	FColor GetDefaultColor(AActor* Actor);

private:
	TMap<FString, FColor> AnnotationColors; // Store annotation data
};
