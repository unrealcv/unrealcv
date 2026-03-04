// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CommandDispatcher.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAliasMultiCommandTest,
    "UnrealCV.Server.CommandDispatcher.AliasMultiCommand",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAliasMultiCommandTest::RunTest(const FString& Parameters)
{
    FCommandDispatcher Dispatcher;

    // Register two simple commands
    int32 CallCount = 0;
    FDispatcherDelegate Cmd;
    Cmd.BindLambda([&CallCount](const TArray<FString>&) {
        ++CallCount;
        return FExecStatus::OK(FString::Printf(TEXT("call%d"), CallCount));
    });
    Dispatcher.BindCommand(TEXT("vget /aliasmc/a"), Cmd, TEXT("a"));
    Dispatcher.BindCommand(TEXT("vget /aliasmc/b"), Cmd, TEXT("b"));

    // Register a multi-command alias
    TArray<FString> Commands;
    Commands.Add(TEXT("vget /aliasmc/a"));
    Commands.Add(TEXT("vget /aliasmc/b"));
    Dispatcher.Alias(TEXT("both"), Commands, TEXT("Run both"));

    // Execute the alias via vrun
    const FExecStatus Result = Dispatcher.Exec(TEXT("vrun both"));
    TestEqual(TEXT("Both commands executed"), CallCount, 2);
    TestTrue(TEXT("Result contains call1"), Result.GetMessage().Contains(TEXT("call1")));
    TestTrue(TEXT("Result contains call2"), Result.GetMessage().Contains(TEXT("call2")));
    return true;
}
