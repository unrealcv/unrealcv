#pragma once
#include "CommandDispatcher.h"

class FCommandHandler
{
public:
	FCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher) :
		Character(InCharacter), CommandDispatcher(InCommandDispatcher)
	{}
	virtual void RegisterCommands() = 0;
protected:
	FCommandDispatcher* CommandDispatcher;
	APawn* Character;
};


class FObjectCommandHandler : public FCommandHandler
{
public:
	FObjectCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCharacter, InCommandDispatcher)
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
};

class FCameraCommandHandler : public FCommandHandler
{
public:
	FCameraCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCharacter, InCommandDispatcher)
	{}
	void RegisterCommands();

	/** vget /camera/location */
	FExecStatus GetCameraLocation(const TArray<FString>& Args);
	/** vset /camera/location */
	FExecStatus	SetCameraLocation(const TArray<FString>& Args);
	/** vget /camera/rotation */
	FExecStatus GetCameraRotation(const TArray<FString>& Args);
	/** vset /camera/rotation */
	FExecStatus	SetCameraRotation(const TArray<FString>& Args);
	/** vget /camera/view */
	FExecStatus GetCameraView(const TArray<FString>& Args);

	/** Deprecated: Get camera view in a sync way, can not work in standalone mode */
	FExecStatus GetCameraViewSync(const FString& Fullfilename);
	/** Get camera view in async way, the return FExecStaus is in pending status and need to check the promise to get result */
	FExecStatus GetCameraViewAsyncQuery(const FString& Fullfilename);
	// FExecStatus GetCameraViewAsyncCallback(const FString& Fullfilename);

	/** Get camera image with a given mode */
	FExecStatus GetCameraViewMode(const TArray<FString>& Args);
};

class FPluginCommandHandler : public FCommandHandler
{
public:
	FPluginCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher) : FCommandHandler(InCharacter, InCommandDispatcher)
	{}
	void RegisterCommands();

	FExecStatus GetPort(const TArray<FString>& Args);
	FExecStatus SetPort(const TArray<FString>& Args);
	FExecStatus GetUnrealCVStatus(const TArray<FString>& Args);
};
