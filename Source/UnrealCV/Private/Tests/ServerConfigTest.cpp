// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "ServerConfig.h"

#if WITH_AUTOMATION_WORKER

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FServerConfigDefaultsTest,
	"UnrealCV.Server.ServerConfig.Defaults",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FServerConfigDefaultsTest::RunTest(const FString& Parameters)
{
	// Use side-effect-free init for deterministic test behaviour.
	FServerConfig Config(EServerConfigInitMode::NoSideEffects);

	TestTrue(TEXT("Default port > 0"), Config.Port > 0);
	TestTrue(TEXT("Default width > 0"), Config.Width > 0);
	TestTrue(TEXT("Default height > 0"), Config.Height > 0);
	TestTrue(TEXT("Default FOV > 0"), Config.FOV > 0.f);
	TestTrue(TEXT("SupportedModes not empty"), Config.SupportedModes.Num() > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FServerConfigToStringTest,
	"UnrealCV.Server.ServerConfig.ToString",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FServerConfigToStringTest::RunTest(const FString& Parameters)
{
	FServerConfig Config(EServerConfigInitMode::NoSideEffects);
	const FString Str = Config.ToString();

	TestTrue(TEXT("ToString contains Port"), Str.Contains(TEXT("Port")));
	TestTrue(TEXT("ToString contains Width"), Str.Contains(TEXT("Width")));
	TestTrue(TEXT("ToString contains Height"), Str.Contains(TEXT("Height")));
	TestTrue(TEXT("ToString contains FOV"), Str.Contains(TEXT("FOV")));

	return true;
}

#endif // WITH_AUTOMATION_WORKER
