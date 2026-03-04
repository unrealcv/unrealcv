// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CommandDispatcher.h"
#include "Commands/AliasHandler.h"

#if WITH_AUTOMATION_WORKER

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAliasVRunCompatibilityTest,
    "UnrealCV.Server.CommandDispatcher.AliasVRunCompatibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAliasVRunCompatibilityTest::RunTest(const FString& Parameters)
{
    TSharedPtr<FCommandDispatcher> Dispatcher = MakeShared<FCommandDispatcher>();

    // Register the command group that used to overwrite "vrun [str]".
    FAliasHandler AliasHandler;
    AliasHandler.SetCommandDispatcher(Dispatcher);
    AliasHandler.RegisterCommands();

    int32 CallCount = 0;
    FDispatcherDelegate Cmd;
    Cmd.BindLambda([&CallCount](const TArray<FString>&) {
        ++CallCount;
        return FExecStatus::OK(TEXT("ok"));
    });
    Dispatcher->BindCommand(TEXT("vget /compat/a"), Cmd, TEXT("a"));
    Dispatcher->Alias(TEXT("compat_alias"), TEXT("vget /compat/a"), TEXT("compat"));

    // If vrun dispatch is intact, this should execute AliasHelper and run /compat/a once.
    const FExecStatus Result = Dispatcher->Exec(TEXT("vrun compat_alias"));
    TestEqual(TEXT("Alias command executed once"), CallCount, 1);
    TestTrue(TEXT("vrun alias returns success"), Result == EExecStatusType::OK);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FAliasErrorPropagationTest,
    "UnrealCV.Server.CommandDispatcher.AliasErrorPropagation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAliasErrorPropagationTest::RunTest(const FString& Parameters)
{
    FCommandDispatcher Dispatcher;

    FDispatcherDelegate OkCmd;
    OkCmd.BindLambda([](const TArray<FString>&) {
        return FExecStatus::OK(TEXT("ok_step"));
    });

    FDispatcherDelegate FailCmd;
    FailCmd.BindLambda([](const TArray<FString>&) {
        return FExecStatus::Error(TEXT("failing_step"));
    });

    Dispatcher.BindCommand(TEXT("vget /aliaserr/ok"), OkCmd, TEXT("ok"));
    Dispatcher.BindCommand(TEXT("vget /aliaserr/fail"), FailCmd, TEXT("fail"));

    TArray<FString> Commands;
    Commands.Add(TEXT("vget /aliaserr/ok"));
    Commands.Add(TEXT("vget /aliaserr/fail"));
    Dispatcher.Alias(TEXT("mixed"), Commands, TEXT("mixed"));

    const FExecStatus Result = Dispatcher.Exec(TEXT("vrun mixed"));
    TestTrue(TEXT("Alias with one failing subcommand returns error"), Result == EExecStatusType::Error);
    TestTrue(TEXT("Combined message keeps both sub-results"),
        Result.GetMessage().Contains(TEXT("ok_step")) && Result.GetMessage().Contains(TEXT("failing_step")));
    return true;
}

#endif // WITH_AUTOMATION_WORKER
