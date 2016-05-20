// Fill out your copyright notice in the Description page of Project Settings.

// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
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

	IConsoleObject* VRunCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vrun"),
		TEXT("Exec alias"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VRun)
		);
}

FConsoleHelper::~FConsoleHelper()
{
}

void FConsoleHelper::VRun(const TArray<FString>& Args)
{
	// Provide support to alias
	if (Args.Num() == 1)
	{
		FString Alias = Args[0];
		FString Cmd = FString::Printf(TEXT("vrun %s"), *Alias);
		FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
		ConsoleOutputDevice->Log(ExecStatus.GetMessage());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Alias can not support extra parameters"));
	}
}

void FConsoleHelper::VGet(const TArray<FString>& Args)
{
	// TODO: Is there any way to know which command trigger this handler?
	// Join string
	FString Cmd = "vget ";
	uint32 NumArgs = Args.Num();
	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; // Maybe a more elegant implementation for joining string
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	UE_LOG(LogTemp, Warning, TEXT("vget helper function, the real command is %s"), *Cmd);
	// In the console mode, output should be writen to the output log.
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	ConsoleOutputDevice->Log(ExecStatus.GetMessage());
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
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	ConsoleOutputDevice->Log(ExecStatus.GetMessage());
}

