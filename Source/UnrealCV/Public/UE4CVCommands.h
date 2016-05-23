// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommandDispatcher.h"

/**
 * Define a set of commands to interact with the Unreal World 
 */
class UNREALCV_API UE4CVCommands
{
private:
	APawn* Character;
	FCommandDispatcher* CommandDispatcher; // Maintain a FCommandDispatcher, so that command can invoke other commands
	void RegisterCommands();
	void RegisterCommandsCamera();

	FExecStatus GetCameraLocation(const TArray<FString>& Args);
	FExecStatus	SetCameraLocation(const TArray<FString>& Args);
	FExecStatus GetCameraRotation(const TArray<FString>& Args);
	FExecStatus	SetCameraRotation(const TArray<FString>& Args);
	FExecStatus GetCameraView(const TArray<FString>& Args);

	FExecStatus GetCameraViewSync(const FString& Fullfilename);
	FExecStatus GetCameraViewAsyncQuery(const FString& Fullfilename);
	// FExecStatus GetCameraViewAsyncCallback(const FString& Fullfilename);

	FExecStatus GetCameraImage(const TArray<FString>& Args);
	FExecStatus GetCameraDepth(const TArray<FString>& Args);
	FExecStatus GetCameraObjectMask(const TArray<FString>& Args);
	FExecStatus GetCameraNormal(const TArray<FString>& Args); // Consider a more compact implementation
	FExecStatus GetCameraBaseColor(const TArray<FString>& Args);
	FExecStatus GetCameraViewMode(const TArray<FString>& Args);

	FExecStatus GetObjects(const TArray<FString>& Args);
	FExecStatus GetObjectColor(const TArray<FString>& Args);
	FExecStatus SetObjectColor(const TArray<FString>& Args);
	FExecStatus GetObjectName(const TArray<FString>& Args);

	FExecStatus CurrentObjectHandler(const TArray<FString>& Args);
	FExecStatus GetCommands(const TArray<FString>& Args);
public:
	UE4CVCommands(APawn* MyCharacter, FCommandDispatcher* InCommandDispatcher);
	~UE4CVCommands();

};
