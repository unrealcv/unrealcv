// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once
#include "CommandHandler.h"

/**
 * Handle vget/vset /object/ commands.
 *
 * "Object" means any actor containing a MeshComponent at runtime
 * (StaticMesh, SkeletalMesh, ProceduralMesh, Landscape, Foliage).
 */
class FObjectHandler : public FCommandHandler
{
public:
	void RegisterCommands() override;

private:
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
	FExecStatus SetHide(const TArray<FString>& Args);
	FExecStatus GetMobility(const TArray<FString>& Args);
	FExecStatus GetObjectVertexLocation(const TArray<FString>& Args);
	FExecStatus Destroy(const TArray<FString>& Args);
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
