// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "ConsoleSettings.h"
#include "Engine/Console.h"

#if WITH_AUTOMATION_WORKER

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FConsoleHelperAutoCompleteTest, "UnrealCV.Server.ConsoleHelper.AutoComplete",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FConsoleHelperAutoCompleteTest::RunTest(const FString& Parameters)
{
    TArray<FAutoCompleteCommand> Entries;
    UConsole::RegisterConsoleAutoCompleteEntries.Broadcast(Entries);

    const auto FindCommand = [&Entries](const FString& Command) -> const FAutoCompleteCommand* {
        return Entries.FindByPredicate([&Command](const FAutoCompleteCommand& Entry)
                                       { return Entry.Command == Command; });
    };

    const FAutoCompleteCommand* Template = FindCommand(TEXT("vget /camera/[uint]/location"));
    TestNotNull(TEXT("Registered command template is suggested"), Template);
    if (Template)
    {
        TestTrue(TEXT("Command help is shown"), Template->Desc.Contains(TEXT("location")));
    }

    TestNotNull(TEXT("Camera 0 example is suggested"), FindCommand(TEXT("vget /camera/0/location")));
    TestNotNull(TEXT("Argument example is suggested"), FindCommand(TEXT("vset /camera/0/fov 0")));

    const auto CountCommand = [&Entries](const FString& Command)
    {
        return Entries
            .FilterByPredicate([&Command](const FAutoCompleteCommand& Entry) { return Entry.Command == Command; })
            .Num();
    };
    const int32 TemplateCountBeforeSecondBroadcast = CountCommand(TEXT("vget /camera/[uint]/location"));
    UConsole::RegisterConsoleAutoCompleteEntries.Broadcast(Entries);
    TestEqual(TEXT("Repeated registration does not duplicate UnrealCV entries"),
              CountCommand(TEXT("vget /camera/[uint]/location")), TemplateCountBeforeSecondBroadcast);

    return true;
}

#endif // WITH_AUTOMATION_WORKER
