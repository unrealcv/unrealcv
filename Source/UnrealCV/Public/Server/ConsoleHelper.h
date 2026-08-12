// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "CommandDispatcher.h"

class FConsoleOutputDevice;
struct FAutoCompleteCommand;

/**
 * Bridges UnrealCV commands to the Unreal Engine console.
 *
 * Registers vget / vset / vrun / vexec / vbp as console commands and
 * forwards them to FCommandDispatcher for execution.
 */
class FConsoleHelper
{
public:
	static FConsoleHelper& Get();
	static FConsoleHelper* GetIfInitialized();
	~FConsoleHelper();

    void SetCommandDispatcher(TSharedPtr<FCommandDispatcher> InCommandDispatcher);
    void UnregisterConsoleAutoComplete();
    void InvalidateConsoleAutoComplete() const;

    /** Obtain a console output device for the current viewport. May return null. */
    [[nodiscard]] TSharedPtr<FConsoleOutputDevice> GetConsole() const;

    // Non-copyable singleton.
    FConsoleHelper(const FConsoleHelper&) = delete;
    FConsoleHelper& operator=(const FConsoleHelper&) = delete;

  private:
    FConsoleHelper();
    void RegisterConsoleAutoComplete();
    void AddAutoCompleteEntries(TArray<FAutoCompleteCommand>& List) const;

    TSharedPtr<FCommandDispatcher> CommandDispatcher;
    FDelegateHandle AutoCompleteDelegateHandle;

    /**
     * Common implementation for all console verbs.
     * Joins Args with the given Prefix and dispatches via CommandDispatcher.
     */
    void DispatchConsoleCommand(const TCHAR* Prefix, const TArray<FString>& Args);

    void VGet(const TArray<FString>& Args);
    void VSet(const TArray<FString>& Args);
    void VRun(const TArray<FString>& Args);
    void VExec(const TArray<FString>& Args);
    void VBp(const TArray<FString>& Args);
};
