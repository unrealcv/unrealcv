// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "Server/SocketUtils.h"

#if WITH_AUTOMATION_WORKER

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSocketUtilsStringRoundtripTest,
	"UnrealCV.Server.SocketUtils.StringRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSocketUtilsStringRoundtripTest::RunTest(const FString& Parameters)
{
	const FString Original = TEXT("Hello, UnrealCV!");

	TArray<uint8> Binary;
	UCV::SocketUtils::BinaryArrayFromString(Original, Binary);
	TestTrue(TEXT("Binary array is not empty"), Binary.Num() > 0);

	const FString Roundtrip = UCV::SocketUtils::StringFromBinaryArray(Binary);
	TestEqual(TEXT("Roundtrip matches original"), Roundtrip, Original);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSocketUtilsEmptyStringTest,
	"UnrealCV.Server.SocketUtils.EmptyString",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSocketUtilsEmptyStringTest::RunTest(const FString& Parameters)
{
	const FString Empty = TEXT("");

	TArray<uint8> Binary;
	UCV::SocketUtils::BinaryArrayFromString(Empty, Binary);
	TestEqual(TEXT("Empty string produces empty binary"), Binary.Num(), 0);

	const FString Roundtrip = UCV::SocketUtils::StringFromBinaryArray(Binary);
	TestEqual(TEXT("Empty roundtrip"), Roundtrip, Empty);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSocketUtilsUnicodeTest,
	"UnrealCV.Server.SocketUtils.Unicode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSocketUtilsUnicodeTest::RunTest(const FString& Parameters)
{
	// Test with common non-ASCII characters.
	const FString Unicode = TEXT("Caf\u00E9 \u00FCber \u00E4hnlich");

	TArray<uint8> Binary;
	UCV::SocketUtils::BinaryArrayFromString(Unicode, Binary);
	TestTrue(TEXT("Unicode produces binary"), Binary.Num() > 0);

	const FString Roundtrip = UCV::SocketUtils::StringFromBinaryArray(Binary);
	TestEqual(TEXT("Unicode roundtrip"), Roundtrip, Unicode);

	return true;
}

#endif // WITH_AUTOMATION_WORKER
