// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommandDispatcher.h"

/**
 * Define a set of commands to interact with the Unreal World 
 */
class UNREALCV_API UE4CVCommands
{
private:
	/** Store a Character pointer, so a command can control the character */
	APawn* Character;
	/** Store a FCommandDispatcher pointer, so a command can invoke other commands */
	FCommandDispatcher* CommandDispatcher; 
	/** Register commands to a CommandDispatcher */
	void RegisterCommands();
	void RegisterCommandsCamera();
	void RegisterCommandsPlugin(); // TODO: make this more extensible

	// See more details in RegisterCommands()
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

	/** Get a list of all objects in the scene */
	FExecStatus GetObjects(const TArray<FString>& Args);
	/** Get the annotation color of an object (Notice: not the appearance color) */
	FExecStatus GetObjectColor(const TArray<FString>& Args);
	/** Set the annotation color of an object */
	FExecStatus SetObjectColor(const TArray<FString>& Args);
	/** Get the name of an object */
	FExecStatus GetObjectName(const TArray<FString>& Args);


	FExecStatus GetPort(const TArray<FString>& Args);
	FExecStatus SetPort(const TArray<FString>& Args);
	FExecStatus GetUnrealCVStatus(const TArray<FString>& Args);

	FExecStatus CurrentObjectHandler(const TArray<FString>& Args);

	/** Get the help message of defined commands */
	FExecStatus GetCommands(const TArray<FString>& Args);
public:
	UE4CVCommands(APawn* MyCharacter, FCommandDispatcher* InCommandDispatcher);
	~UE4CVCommands();
};
