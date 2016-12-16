#include "UnrealCVPrivate.h"
#include "UE4CVServer.h"
#include "ConsoleHelper.h"

FConsoleHelper::FConsoleHelper()
{
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
		// TEXT("Exec alias"),
		TEXT("Exec Unreal Engine commands"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VRun)
		);
}

FConsoleHelper& FConsoleHelper::Get()
{
	static FConsoleHelper Singleton;
	return Singleton;
}

void FConsoleHelper::SetCommandDispatcher(FCommandDispatcher* InCommandDispatcher)
{
	CommandDispatcher = InCommandDispatcher;
}

FConsoleOutputDevice* FConsoleHelper::GetConsole() // The ConsoleOutputDevice will depend on the external world, so we need to use a get function
{
	static FConsoleOutputDevice* ConsoleOutputDevice = nullptr;
	static UWorld* CurrentWorld = nullptr;
	if (ConsoleOutputDevice == nullptr || CurrentWorld != GWorld)
	{
		ConsoleOutputDevice = new FConsoleOutputDevice(GWorld->GetGameViewport()->ViewportConsole); // TODO: Check the pointers
	}
	return ConsoleOutputDevice;
}

void FConsoleHelper::VRun(const TArray<FString>& Args)
{
	if (CommandDispatcher == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("CommandDispatcher not set"));
	}
	FString Cmd = "vrun ";
	uint32 NumArgs = Args.Num();
	if (NumArgs == 0) return;

	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; // Maybe a more elegant implementation for joining string
	FUE4CVServer::Get().InitWorld();
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	UE_LOG(LogUnrealCV, Warning, TEXT("vrun helper function, the real command is %s"), *Cmd);
	// In the console mode, output should be writen to the output log.
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	GetConsole()->Log(ExecStatus.GetMessage());
}

void FConsoleHelper::VGet(const TArray<FString>& Args)
{
	if (CommandDispatcher == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("CommandDispatcher not set"));
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
	FUE4CVServer::Get().InitWorld();
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	UE_LOG(LogUnrealCV, Warning, TEXT("vget helper function, the real command is %s"), *Cmd);
	// In the console mode, output should be writen to the output log.
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	GetConsole()->Log(ExecStatus.GetMessage());
}

void FConsoleHelper::VSet(const TArray<FString>& Args)
{
	if (CommandDispatcher == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("CommandDispatcher not set"));
	}
	FString Cmd = "vset ";
	uint32 NumArgs = Args.Num();
	if (NumArgs == 0) return;

	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1];
	FUE4CVServer::Get().InitWorld();
	FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);
	// Output result to the console
	UE_LOG(LogUnrealCV, Warning, TEXT("vset helper function, the real command is %s"), *Cmd);
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ExecStatus.GetMessage());
	GetConsole()->Log(ExecStatus.GetMessage());
}
