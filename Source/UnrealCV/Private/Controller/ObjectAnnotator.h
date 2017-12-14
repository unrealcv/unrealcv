// Weichao Qiu @ 2017
#pragma once

/** Annotate each object instance with a unique color */
class FObjectAnnotator
{
public:
	FObjectAnnotator();

    /** Annotate all StaticMesh actor in the world */
    void AnnotateStaticMesh();

    void SetObjectColor(FString ObjId, const FColor& AnnotationColor);

    void SetObjectColor(AActor* Actor, const FColor& AnnotationColor);

    void GetObjectColor(FString ObjId, FColor& AnnotationColor);

    void GetObjectColor(AActor* Actor, FColor& AnnotationColor);

    /** Dump the annotation for each object */
    void SaveAnnotation(FString JsonFilename);

    /** Load the annotation for each object */
    void LoadAnnotation(FString JsonFilename);

private:
    TMap<FString, FColor> AnnotationColors;

    /** Assign a unique new color for this object */
	void InitObjInstanceColor(AActor* Actor, FColor& AnnotationColor);
};
