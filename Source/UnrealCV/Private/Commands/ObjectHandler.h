#pragma once
#include "CommandDispatcher.h"

class FObjectCommandHandler : public FCommandHandler
{
public:
	FObjectCommandHandler(FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCommandDispatcher)
	{}
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
