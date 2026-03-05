// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once
#include "CommandHandler.h"

class FAliasHandler : public FCommandHandler
{
public:
	void RegisterCommands() override;

private:
	/** vrun : run UE built-in commands */
	FExecStatus VRun(const TArray<FString>& Args);

	/** vexec : run Blueprint function (no return value) */
	FExecStatus VExec(const TArray<FString>& Args);

	FExecStatus VExecWithOutput(const TArray<FString>& Args);

	FExecStatus GetPersistentLevelId(const TArray<FString>& Args);

	FExecStatus GetLevelScriptActorId(const TArray<FString>& Args);
};
