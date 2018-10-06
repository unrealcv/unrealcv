// Weichao Qiu @ 2016
#pragma once
#include "CommandDispatcher.h"
#include "CommandHandler.h"

/** 
 * Handle vget/vset /object/ commands.
 * The definition of object is any actor contains a MeshComponent in the runtime
 * This includes StaticMesh, SkeletalMesh, ProceduralMesh (tree), Landscape, Foliage
 * Object is different than actor, only actor contains mesh can be called as object
*/
class FObjectHandler : public FCommandHandler
{
public:
	void RegisterCommands();

private:
	/** Get a list of all objects in the scene */
	FExecStatus GetObjectList(const TArray<FString>& Args);

	/** Get the annotation color of an object (Notice: not the appearance color) */
	FExecStatus GetObjectAnnotationColor(const TArray<FString>& Args);

	/** Set the annotation color of an object */
	FExecStatus SetObjectAnnotationColor(const TArray<FString>& Args);

	/** Get the name of an object */
	FExecStatus GetObjectName(const TArray<FString>& Args);

	FExecStatus CurrentObjectHandler(const TArray<FString>& Args);

	/** Get object location */
	FExecStatus GetObjectLocation(const TArray<FString>& Args);

	/** Get object rotation */
	FExecStatus GetObjectRotation(const TArray<FString>& Args);

	/** Set object location */
	FExecStatus SetObjectLocation(const TArray<FString>& Args);

	/** Set object rotation */
	FExecStatus SetObjectRotation(const TArray<FString>& Args);

	/** Show object */
	FExecStatus SetShowObject(const TArray<FString>& Args);

	/** Hide object */
	FExecStatus SetHideObject(const TArray<FString>& Args);

	FExecStatus GetObjectMobility(const TArray<FString>& Args);

	FExecStatus GetObjectVertexLocation(const TArray<FString>& Args);
};
