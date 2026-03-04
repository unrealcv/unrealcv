// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "UnrealcvLog.h"

#if WITH_AUTOMATION_WORKER

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FLoggerLogOnceTest,
	"UnrealCV.Server.Logger.LogOnce",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FLoggerLogOnceTest::RunTest(const FString& Parameters)
{
	FUnrealcvLogger Logger;

	// First call should log; second should suppress.
	// We can't easily verify log output, but we verify it doesn't crash.
	Logger.LogOnce(TEXT("Test message A"));
	Logger.LogOnce(TEXT("Test message A"));  // Should be suppressed
	Logger.LogOnce(TEXT("Test message B"));  // Should not be suppressed

	// After reset, same message should log again.
	Logger.ResetSeenMessages();
	Logger.LogOnce(TEXT("Test message A"));  // Should log again after reset

	return true;
}

#endif // WITH_AUTOMATION_WORKER
