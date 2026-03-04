// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "CommandDispatcher.h"

#if WITH_AUTOMATION_WORKER

namespace
{
	/** Simple command that echoes the first argument. */
	FExecStatus EchoCommand(const TArray<FString>& Args)
	{
		if (Args.Num() == 0) { return FExecStatus::Error(TEXT("no args")); }
		return FExecStatus::OK(Args[0]);
	}

	/** Command that returns the argument count. */
	FExecStatus CountCommand(const TArray<FString>& Args)
	{
		return FExecStatus::OK(FString::FromInt(Args.Num()));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDispatcherBindAndExecTest,
	"UnrealCV.Server.CommandDispatcher.BindAndExec",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDispatcherBindAndExecTest::RunTest(const FString& Parameters)
{
	FCommandDispatcher Dispatcher;

	FDispatcherDelegate Cmd;
	Cmd.BindStatic(&EchoCommand);
	const bool Bound = Dispatcher.BindCommand(TEXT("vget /test/echo [str]"), Cmd, TEXT("Echo test"));
	TestTrue(TEXT("BindCommand succeeds"), Bound);

	// Execute on game thread (these tests run on game thread)
	const FExecStatus Status = Dispatcher.Exec(TEXT("vget /test/echo hello"));
	TestEqual(TEXT("Echo returns argument"), Status.GetMessage(), TEXT("hello"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDispatcherNoMatchTest,
	"UnrealCV.Server.CommandDispatcher.NoMatch",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDispatcherNoMatchTest::RunTest(const FString& Parameters)
{
	FCommandDispatcher Dispatcher;

	const FExecStatus Status = Dispatcher.Exec(TEXT("vget /nonexistent"));
	TestTrue(TEXT("No match returns Error"), Status == EExecStatusType::Error);
	TestTrue(TEXT("Error message mentions URI"), Status.GetMessage().Contains(TEXT("No handler")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDispatcherMultiArgTest,
	"UnrealCV.Server.CommandDispatcher.MultiArg",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDispatcherMultiArgTest::RunTest(const FString& Parameters)
{
	FCommandDispatcher Dispatcher;

	FDispatcherDelegate Cmd;
	Cmd.BindStatic(&CountCommand);
	Dispatcher.BindCommand(TEXT("vget /test/count [uint] [str]"), Cmd, TEXT("Count test"));

	const FExecStatus Status = Dispatcher.Exec(TEXT("vget /test/count 42 hello"));
	TestEqual(TEXT("Two arguments captured"), Status.GetMessage(), TEXT("2"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDispatcherDescriptionTest,
	"UnrealCV.Server.CommandDispatcher.Description",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDispatcherDescriptionTest::RunTest(const FString& Parameters)
{
	FCommandDispatcher Dispatcher;

	FDispatcherDelegate Cmd;
	Cmd.BindStatic(&EchoCommand);
	Dispatcher.BindCommand(TEXT("vget /test/desc [str]"), Cmd, TEXT("My description"));

	const auto& Descriptions = Dispatcher.GetUriDescription();
	const FString* Desc = Descriptions.Find(TEXT("vget /test/desc [str]"));
	TestNotNull(TEXT("Description registered"), Desc);
	if (Desc) { TestEqual(TEXT("Description value"), *Desc, TEXT("My description")); }

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDispatcherAliasTest,
	"UnrealCV.Server.CommandDispatcher.Alias",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDispatcherAliasTest::RunTest(const FString& Parameters)
{
	FCommandDispatcher Dispatcher;

	FDispatcherDelegate Cmd;
	Cmd.BindStatic(&EchoCommand);
	Dispatcher.BindCommand(TEXT("vget /test/echo [str]"), Cmd, TEXT("Echo"));

	const bool AliasOk = Dispatcher.Alias(TEXT("myalias"), TEXT("vget /test/echo aliased"), TEXT("Alias test"));
	TestTrue(TEXT("Alias registration succeeds"), AliasOk);

	const FExecStatus Status = Dispatcher.Exec(TEXT("vrun myalias"));
	TestTrue(TEXT("Alias resolves"), Status.GetMessage().Contains(TEXT("aliased")));

	return true;
}

#endif // WITH_AUTOMATION_WORKER
