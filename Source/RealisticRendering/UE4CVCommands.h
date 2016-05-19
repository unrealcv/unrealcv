// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CommandDispatcher.h"
#include "MyCharacter.h"
/**
 * 
 */
class REALISTICRENDERING_API UE4CVCommands
{
	friend class AMyCharacter; // Need to access information from Character;
private:
	AMyCharacter* Character;
	FCommandDispatcher* CommandDispatcher;
	void RegisterCommands();

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

	FExecStatus GetObjects(const TArray<FString>& Args);
	FExecStatus GetObjectColor(const TArray<FString>& Args);
	FExecStatus SetObjectColor(const TArray<FString>& Args);
	FExecStatus GetObjectName(const TArray<FString>& Args);

	FExecStatus CurrentObjectHandler(const TArray<FString>& Args);
	FExecStatus GetCommands(const TArray<FString>& Args);
public:
	UE4CVCommands(AMyCharacter* MyCharacter);
	~UE4CVCommands();

};
