// Weichao Qiu @ 2016
#pragma once
#include "CommandDispatcher.h"
#include "CommandHandler.h"

/** 
 * Handle vget/vset /object/... commands 
 * The definition of object is any actor contains a MeshComponent in the runtime
 * This includes StaticMesh, SkeletalMesh, ProceduralMesh (tree), Landscape, Foliage
 * Object is different than actor, only actor contains mesh can be called as object
*/
class FObjectCommandHandler : public FCommandHandler
{
public:
	void RegisterCommands();

	/** Get a list of all objects in the scene */
	FExecStatus GetObjects(const TArray<FString>& Args);
	/** Get the annotation color of an object (Notice: not the appearance color) */
	FExecStatus GetObjectColor(const TArray<FString>& Args);
	/** Set the annotation color of an object */
	FExecStatus SetObjectColor(const TArray<FString>& Args);
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
	FExecStatus ShowObject(const TArray<FString>& Args);

	/** Hide object */
	FExecStatus HideObject(const TArray<FString>& Args);
};
