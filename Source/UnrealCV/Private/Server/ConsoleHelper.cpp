// Weichao Qiu @ 2016
#include "ConsoleHelper.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "UnrealcvServer.h"
#include "UnrealcvLog.h"

void FConsoleHelper::VBp(const TArray<FString>& Args)
{
	if (!CommandDispatcher.IsValid())
	{
		UE_LOG(LogUnrealCV, Error, TEXT("CommandDispatcher not set"));
	}
	FString Cmd = "vbp ";
	uint32 NumArgs = Args.Num();
	if (NumArgs == 0) return;

	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1];
	
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	UE_LOG(LogUnrealCV, Warning, TEXT("vbp helper function, the real command is %s"), *Cmd);
	// In the console mode, output should be writen to the output log.
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	GetConsole()->Log(ExecStatus.GetMessage());
}

FConsoleHelper::FConsoleHelper()
{
	// Add Unreal Console Support
	IConsoleObject* VGetCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vget"),
		TEXT("Get resource from Unreal Engine"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VGet)
		);

	IConsoleObject* VSetCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vset"),
		TEXT("Set resource in Unreal Engine"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VSet)
		);

	IConsoleObject* VRunCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vrun"),
		TEXT("Exec Unreal Engine commands"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VRun)
		);

	IConsoleObject* VExecCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vexec"),
		TEXT("Exec Blueprint Function"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VExec)
		);

	// TODO: Simplify this file
	IConsoleObject* VBpCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vbp"),
		TEXT("Exec Blueprint Function"),
		// FConsoleCommandWithArgsDelegate::CreateStatic(VBp)
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VBp)
		);
}

FConsoleHelper& FConsoleHelper::Get()
{
	static FConsoleHelper Singleton;
	return Singleton;
}

void FConsoleHelper::SetCommandDispatcher(TSharedPtr<FCommandDispatcher> InCommandDispatcher)
{
	CommandDispatcher = InCommandDispatcher;
}

TSharedPtr<FConsoleOutputDevice> FConsoleHelper::GetConsole() // The ConsoleOutputDevice will depend on the external world, so we need to use a get function
{
	TSharedPtr<FConsoleOutputDevice> ConsoleOutputDevice(new FConsoleOutputDevice(FUnrealcvServer::Get().GetWorld()->GetGameViewport()->ViewportConsole));
	return ConsoleOutputDevice;
}

void FConsoleHelper::VRun(const TArray<FString>& Args)
{
	if (!CommandDispatcher.IsValid())
	{
		UE_LOG(LogUnrealCV, Error, TEXT("CommandDispatcher not set"));
		return;
	}
	FString Cmd = "vrun ";
	uint32 NumArgs = Args.Num();
	if (NumArgs == 0) return;

	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; // Maybe a more elegant implementation for joining string
	// FUnrealcvServer::Get().InitWorld();
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	UE_LOG(LogUnrealCV, Warning, TEXT("vrun helper function, the real command is %s"), *Cmd);
	// In the console mode, output should be writen to the output log.
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	GetConsole()->Log(ExecStatus.GetMessage());
}

void FConsoleHelper::VGet(const TArray<FString>& Args)
{
	if (!CommandDispatcher.IsValid())
	{
		UE_LOG(LogUnrealCV, Error, TEXT("CommandDispatcher not set"));
		return;
	}
	// TODO: Is there any way to know which command trigger this handler?
	// Join string
	FString Cmd = "vget ";
	uint32 NumArgs = Args.Num();
	if (NumArgs == 0) return;

	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; // Maybe a more elegant implementation for joining string
	// FUnrealcvServer::Get().InitWorld();
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	UE_LOG(LogUnrealCV, Warning, TEXT("vget helper function, the real command is %s"), *Cmd);
	// In the console mode, output should be writen to the output log.
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	GetConsole()->Log(ExecStatus.GetMessage());
}

void FConsoleHelper::VSet(const TArray<FString>& Args)
{
	if (!CommandDispatcher.IsValid())
	{
		UE_LOG(LogUnrealCV, Error, TEXT("CommandDispatcher not set"));
		return;
	}
	FString Cmd = "vset ";
	uint32 NumArgs = Args.Num();
	if (NumArgs == 0) return;

	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1];
	// FUnrealcvServer::Get().InitWorld();
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	// Output result to the console
	UE_LOG(LogUnrealCV, Warning, TEXT("vset helper function, the real command is %s"), *Cmd);
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	GetConsole()->Log(ExecStatus.GetMessage());
}

void FConsoleHelper::VExec(const TArray<FString>& Args)
{
	FString Cmd = "vexec ";
	uint32 NumArgs = Args.Num();
	if (NumArgs == 0) return;

	for (uint32 ArgIndex = 0; ArgIndex < NumArgs - 1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs - 1];

	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	GetConsole()->Log(ExecStatus.GetMessage());
}
