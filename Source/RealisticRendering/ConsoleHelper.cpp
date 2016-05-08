// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "ConsoleHelper.h"

FConsoleHelper::FConsoleHelper(FCommandDispatcher* CommandDispatcher, FConsoleOutputDevice* ConsoleOutputDevice)
{
	this->CommandDispatcher = CommandDispatcher;
	this->ConsoleOutputDevice = ConsoleOutputDevice;
	// Add Unreal Console Support
	IConsoleObject* VGetCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vget"),
		TEXT("Set resource in Unreal Engine"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VGet)
		);

	IConsoleObject* VSetCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vset"),
		TEXT("Set resource in Unreal Engine"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VSet)
		);
}

FConsoleHelper::~FConsoleHelper()
{
}

void FConsoleHelper::VGet(const TArray<FString>& Args)
{
	// Join string
	FString Cmd = "vget ";
	uint32 NumArgs = Args.Num();
	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; // Maybe a more elegant implementation
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	UE_LOG(LogTemp, Warning, TEXT("vget helper function, the real command is %s"), *Cmd);
	// In the console mode, output should be writen to the output log.
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ExecStatus.Message);
	ConsoleOutputDevice->Log(ExecStatus.Message);
}

void FConsoleHelper::VSet(const TArray<FString>& Args)
{
	FString Cmd = "vset ";
	uint32 NumArgs = Args.Num();
	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; 
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	// Output result to the console
	UE_LOG(LogTemp, Warning, TEXT("vset helper function, the real command is %s"), *Cmd);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ExecStatus.Message);
	ConsoleOutputDevice->Log(ExecStatus.Message);
}

