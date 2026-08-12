// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "ConsoleHelper.h"
#include "ConsoleSettings.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/Engine/Public/EngineUtils.h"
#include "UnrealcvServer.h"
#include "UnrealcvLog.h"

namespace
{
FConsoleHelper* GConsoleHelper = nullptr;

void AddAutoCompleteCommand(TArray<FAutoCompleteCommand>& List, const FString& Command, const FString& Description)
{
    if (List.ContainsByPredicate([&Command](const FAutoCompleteCommand& Existing)
                                 { return Existing.Command == Command; }))
    {
        return;
    }

    const UConsoleSettings* ConsoleSettings = GetDefault<UConsoleSettings>();
    FAutoCompleteCommand& Entry = List.AddDefaulted_GetRef();
    Entry.Command = Command;
    Entry.Desc = Description;
    Entry.Color = ConsoleSettings ? ConsoleSettings->AutoCompleteCommandColor : FColor(180, 180, 180);
}

FString GetFirstHelpLine(const FString& Description)
{
    TArray<FString> Lines;
    Description.ParseIntoArrayLines(Lines, false);
    return Lines.IsEmpty() ? FString() : Lines[0];
}

FString ReplaceArgumentPlaceholdersWithExamples(const FString& CommandTemplate)
{
    FString Command = CommandTemplate;
    Command.ReplaceInline(TEXT("[Anything]"), TEXT("value"));
    Command.ReplaceInline(TEXT("[str]"), TEXT("name"));
    Command.ReplaceInline(TEXT("[uint]"), TEXT("0"));
    Command.ReplaceInline(TEXT("[float]"), TEXT("0"));
    Command.ReplaceInline(TEXT("[bool]"), TEXT("true"));
    return Command;
}

void AddGeneratedAutoCompleteCommands(TArray<FAutoCompleteCommand>& List, const FString& CommandTemplate,
                                      const FString& Description)
{
    const FString FirstHelpLine = GetFirstHelpLine(Description);
    AddAutoCompleteCommand(List, CommandTemplate, FirstHelpLine);
    AddAutoCompleteCommand(List, ReplaceArgumentPlaceholdersWithExamples(CommandTemplate), FirstHelpLine);

    if (CommandTemplate.Contains(TEXT("/camera/[uint]")))
    {
        for (int32 CameraIndex = 0; CameraIndex < 4; ++CameraIndex)
        {
            FString CameraCommand =
                CommandTemplate.Replace(TEXT("/camera/[uint]"), *FString::Printf(TEXT("/camera/%d"), CameraIndex));
            AddAutoCompleteCommand(List, ReplaceArgumentPlaceholdersWithExamples(CameraCommand), FirstHelpLine);
        }
    }
}
} // namespace

// ---------------------------------------------------------------------------
// Singleton
// ---------------------------------------------------------------------------

FConsoleHelper& FConsoleHelper::Get()
{
	static FConsoleHelper Singleton;
	GConsoleHelper = &Singleton;
	return Singleton;
}

FConsoleHelper* FConsoleHelper::GetIfInitialized()
{
	return GConsoleHelper;
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

    RegisterConsoleAutoComplete();
}

FConsoleHelper::~FConsoleHelper()
{
	UnregisterConsoleAutoComplete();
	GConsoleHelper = nullptr;
}

// ---------------------------------------------------------------------------
// Setters / Getters
// ---------------------------------------------------------------------------

void FConsoleHelper::SetCommandDispatcher(TSharedPtr<FCommandDispatcher> InCommandDispatcher)
{
    CommandDispatcher = InCommandDispatcher;
    InvalidateConsoleAutoComplete();
}

void FConsoleHelper::RegisterConsoleAutoComplete()
{
    if (!AutoCompleteDelegateHandle.IsValid())
    {
        AutoCompleteDelegateHandle =
            UConsole::RegisterConsoleAutoCompleteEntries.AddRaw(this, &FConsoleHelper::AddAutoCompleteEntries);
    }
}

void FConsoleHelper::UnregisterConsoleAutoComplete()
{
    if (AutoCompleteDelegateHandle.IsValid())
    {
        UConsole::RegisterConsoleAutoCompleteEntries.Remove(AutoCompleteDelegateHandle);
        AutoCompleteDelegateHandle.Reset();
    }
}

void FConsoleHelper::InvalidateConsoleAutoComplete() const
{
    if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportConsole)
    {
        GEngine->GameViewport->ViewportConsole->InvalidateAutocomplete();
    }
}

void FConsoleHelper::AddAutoCompleteEntries(TArray<FAutoCompleteCommand>& List) const
{
    if (!CommandDispatcher.IsValid())
    {
        return;
    }

    for (const TPair<FString, FString>& Entry : CommandDispatcher->GetUriDescription())
    {
        AddGeneratedAutoCompleteCommands(List, Entry.Key, Entry.Value);
    }
}

TSharedPtr<FConsoleOutputDevice> FConsoleHelper::GetConsole() const
{
    UWorld* World = FUnrealcvServer::Get().GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UGameViewportClient* Viewport = World->GetGameViewport();
    if (!Viewport || !Viewport->ViewportConsole)
    {
        return nullptr;
    }

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
        UE_LOG(LogUnrealCV, Warning, TEXT("Console command '%s' called with no arguments."), Prefix);
        return;
    }

    FString Cmd = Prefix;
    Cmd += TEXT(" ");
    Cmd += FString::Join(Args, TEXT(" "));

    const FExecStatus ExecStatus = CommandDispatcher->Exec(Cmd);

    UE_LOG(LogUnrealCV, Display, TEXT("[%s] %s -> %s"), Prefix, *Cmd, *ExecStatus.GetMessage());

    TSharedPtr<FConsoleOutputDevice> Console = GetConsole();
    if (Console.IsValid())
    {
        Console->Log(ExecStatus.GetMessage());
    }
}

// ---------------------------------------------------------------------------
// Console verb handlers
// ---------------------------------------------------------------------------

void FConsoleHelper::VGet(const TArray<FString>& Args)
{
    DispatchConsoleCommand(TEXT("vget"), Args);
}
void FConsoleHelper::VSet(const TArray<FString>& Args)
{
    DispatchConsoleCommand(TEXT("vset"), Args);
}
void FConsoleHelper::VRun(const TArray<FString>& Args)
{
    DispatchConsoleCommand(TEXT("vrun"), Args);
}
void FConsoleHelper::VExec(const TArray<FString>& Args)
{
    DispatchConsoleCommand(TEXT("vexec"), Args);
}
void FConsoleHelper::VBp(const TArray<FString>& Args)
{
    DispatchConsoleCommand(TEXT("vbp"), Args);
}
