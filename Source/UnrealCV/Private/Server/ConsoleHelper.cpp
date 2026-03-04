// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "ConsoleHelper.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "UnrealcvServer.h"
#include "UnrealcvLog.h"

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

FConsoleHelper& FConsoleHelper::Get()
{
	static FConsoleHelper Singleton;
	return Singleton;
}

// ---------------------------------------------------------------------------
// Constructor — register console commands
// ---------------------------------------------------------------------------

FConsoleHelper::FConsoleHelper()
{
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vget"), TEXT("Get resource from Unreal Engine"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VGet));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vset"), TEXT("Set resource in Unreal Engine"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VSet));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vrun"), TEXT("Execute UnrealCV alias commands"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VRun));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vexec"), TEXT("Execute Blueprint function"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VExec));

	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vbp"), TEXT("Execute Blueprint function (legacy alias)"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FConsoleHelper::VBp));
}

// ---------------------------------------------------------------------------
// Setters / Getters
// ---------------------------------------------------------------------------

void FConsoleHelper::SetCommandDispatcher(TSharedPtr<FCommandDispatcher> InCommandDispatcher)
{
	CommandDispatcher = InCommandDispatcher;
}

TSharedPtr<FConsoleOutputDevice> FConsoleHelper::GetConsole()
{
	UGameViewportClient* Viewport = FUnrealcvServer::Get().GetWorld()->GetGameViewport();
	return MakeShared<FConsoleOutputDevice>(Viewport->ViewportConsole);
}

// ---------------------------------------------------------------------------
// Shared dispatch logic (eliminates per-verb duplication)
// ---------------------------------------------------------------------------

void FConsoleHelper::DispatchConsoleCommand(const TCHAR* Prefix, const TArray<FString>& Args)
{
	if (!CommandDispatcher.IsValid())
	{
		UE_LOG(LogUnrealCV, Error, TEXT("CommandDispatcher not set — cannot execute console command."));
		return;
	}

	if (Args.Num() == 0)
	{
		return;
	}

	FString Cmd = Prefix;
	Cmd += TEXT(" ");
	Cmd += FString::Join(Args, TEXT(" "));

	const FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);

	UE_LOG(LogUnrealCV, Display, TEXT("[%s] %s -> %s"), Prefix, *Cmd, *ExecStatus.GetMessage());
	GetConsole()->Log(ExecStatus.GetMessage());
}

// ---------------------------------------------------------------------------
// Console verb handlers
// ---------------------------------------------------------------------------

void FConsoleHelper::VGet (const TArray<FString>& Args) { DispatchConsoleCommand(TEXT("vget"),  Args); }
void FConsoleHelper::VSet (const TArray<FString>& Args) { DispatchConsoleCommand(TEXT("vset"),  Args); }
void FConsoleHelper::VRun (const TArray<FString>& Args) { DispatchConsoleCommand(TEXT("vrun"),  Args); }
void FConsoleHelper::VExec(const TArray<FString>& Args) { DispatchConsoleCommand(TEXT("vexec"), Args); }
void FConsoleHelper::VBp  (const TArray<FString>& Args) { DispatchConsoleCommand(TEXT("vbp"),   Args); }
