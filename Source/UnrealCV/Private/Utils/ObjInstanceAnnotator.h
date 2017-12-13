// Weichao Qiu @ 2017

/** Annotate each object instance with a unique color */
class FObjInstanceAnnotator
{
public:
	FObjInstanceAnnotator();

    /** Annotate all StaticMesh actor in the world */
    void AnnotateStaticMesh();

    void SetObjInstanceColor(FString ObjId, const FColor& AnnotationColor);

    void GetObjInstanceColor(FString ObjId, FColor& AnnotationColor);

    /** Dump the annotation for each object */
    void SaveAnnotation(FString JsonFilename);

    /** Load the annotation for each object */
    void LoadAnnotation(FString JsonFilename);

private:
    TMap<FString, FColor> AnnotationColors;

    /** Assign a unique new color for this object */
	void InitObjInstanceColor(AActor* Actor, FColor& AnnotationColor);
};
