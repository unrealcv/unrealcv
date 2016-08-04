#pragma once
#include "CommandDispatcher.h"

class FCommandHandler
{
public:
	FCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher)
	: Character(InCharacter), CommandDispatcher(InCommandDispatcher)
	{}
	virtual void RegisterCommands() {};
	virtual ~FCommandHandler() {};
protected:
	FCommandDispatcher* CommandDispatcher;
	APawn* Character;
};

class FObjectCommandHandler : public FCommandHandler
{
public:
	FObjectCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher)
	: FCommandHandler(InCharacter, InCommandDispatcher)
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

	/** Get the location of an object */
	FExecStatus GetObjectLocation(const TArray<FString>& Args);
};

class FPluginCommandHandler : public FCommandHandler
{
public:
	FPluginCommandHandler(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher)
	: FCommandHandler(InCharacter, InCommandDispatcher)
	{}
	void RegisterCommands();

	FExecStatus GetPort(const TArray<FString>& Args);
	FExecStatus SetPort(const TArray<FString>& Args);
	FExecStatus GetUnrealCVStatus(const TArray<FString>& Args);
	/** Get the help message of defined commands */
	FExecStatus GetCommands(const TArray<FString>& Args);
};
