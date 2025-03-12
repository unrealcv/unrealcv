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
	// See RegisterCommands in ObjectHandler.cpp for what each function is doing.
	FExecStatus GetObjectList(const TArray<FString>& Args);

	FExecStatus SpawnBox(const TArray<FString>& Args);

	FExecStatus Spawn(const TArray<FString>& Args);

	FExecStatus GetAnnotationColor(const TArray<FString>& Args);

	FExecStatus SetAnnotationColor(const TArray<FString>& Args);

	FExecStatus GetObjectName(const TArray<FString>& Args);

	FExecStatus CurrentObjectHandler(const TArray<FString>& Args);

	FExecStatus GetLocation(const TArray<FString>& Args);

	FExecStatus GetRotation(const TArray<FString>& Args);

	FExecStatus SetLocation(const TArray<FString>& Args);

	FExecStatus SetRotation(const TArray<FString>& Args);

	FExecStatus SetShow(const TArray<FString>& Args);

	FExecStatus SpawnBpAsset(const TArray<FString>& Args);

	FExecStatus SetHide(const TArray<FString>& Args);

	FExecStatus GetMobility(const TArray<FString>& Args);

	FExecStatus GetObjectVertexLocation(const TArray<FString>& Args);

	FExecStatus Destroy(const TArray<FString>& Args);

	FExecStatus SetPhysics(const TArray<FString>& Args);

	FExecStatus SetCollision(const TArray<FString>& Args);

	FExecStatus GetCollisionNums(const TArray<FString>& Args);

	FExecStatus SetObjectMobility(const TArray<FString>& Args);

	FExecStatus GetUClassName(const TArray<FString>& Args);

	FExecStatus GetComponents(const TArray<FString>& Args);

	FExecStatus SetName(const TArray<FString>& Args);

#if WITH_EDITOR
	FExecStatus SetActorLabel(const TArray<FString>& Args);

	FExecStatus GetActorLabel(const TArray<FString>& Args);
#endif

	FExecStatus GetScale(const TArray<FString>& Args);

	FExecStatus SetScale(const TArray<FString>& Args);

	FExecStatus GetBounds(const TArray<FString>& Args);
};
