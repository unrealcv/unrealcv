// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CommandDispatcher.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCommandDispatcherMalformedUriTest,
    "UnrealCV.Server.CommandDispatcher.MalformedUri",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCommandDispatcherMalformedUriTest::RunTest(const FString& Parameters)
{
    FCommandDispatcher Dispatcher;
    FDispatcherDelegate Cmd;
    Cmd.BindLambda([](const TArray<FString>&) { return FExecStatus::OK(); });

    // Unclosed bracket
    const bool bUnclosed = Dispatcher.BindCommand(TEXT("vget /test/[str"), Cmd, TEXT("bad"));
    TestFalse(TEXT("Unclosed bracket rejected"), bUnclosed);

    // Unknown type specifier
    const bool bUnknown = Dispatcher.BindCommand(TEXT("vget /test/[foobar]"), Cmd, TEXT("bad"));
    TestFalse(TEXT("Unknown type rejected"), bUnknown);

    // Nested brackets
    const bool bNested = Dispatcher.BindCommand(TEXT("vget /test/[[str]]"), Cmd, TEXT("bad"));
    TestFalse(TEXT("Nested brackets rejected"), bNested);

    // Valid URI still works
    const bool bValid = Dispatcher.BindCommand(TEXT("vget /test/valid [uint]"), Cmd, TEXT("ok"));
    TestTrue(TEXT("Valid URI accepted"), bValid);

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCommandDispatcherExecNotFoundTest,
    "UnrealCV.Server.CommandDispatcher.ExecNotFound",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCommandDispatcherExecNotFoundTest::RunTest(const FString& Parameters)
{
    FCommandDispatcher Dispatcher;
    const FExecStatus Status = Dispatcher.Exec(TEXT("vget /nonexistent"));
    TestTrue(TEXT("Returns error for unknown URI"),
        Status == EExecStatusType::Error);
    TestTrue(TEXT("Error message mentions URI"),
        Status.GetMessage().Contains(TEXT("nonexistent")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FCommandDispatcherOverwriteTest,
    "UnrealCV.Server.CommandDispatcher.Overwrite",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCommandDispatcherOverwriteTest::RunTest(const FString& Parameters)
{
    FCommandDispatcher Dispatcher;
    FDispatcherDelegate Cmd1;
    Cmd1.BindLambda([](const TArray<FString>&) { return FExecStatus::OK(TEXT("first")); });

    FDispatcherDelegate Cmd2;
    Cmd2.BindLambda([](const TArray<FString>&) { return FExecStatus::OK(TEXT("second")); });

    Dispatcher.BindCommand(TEXT("vget /ow/test"), Cmd1, TEXT("first"));
    Dispatcher.BindCommand(TEXT("vget /ow/test"), Cmd2, TEXT("second"));

    const FExecStatus Result = Dispatcher.Exec(TEXT("vget /ow/test"));
    TestEqual(TEXT("Second binding wins"), Result.GetMessage(), TEXT("second"));
    return true;
}
