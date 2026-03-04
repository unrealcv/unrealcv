// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "ExecStatus.h"

#if WITH_AUTOMATION_WORKER

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExecStatusOKTest,
	"UnrealCV.Server.ExecStatus.OK",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusOKTest::RunTest(const FString& Parameters)
{
	// Default OK with no message should return "ok".
	const FExecStatus Status = FExecStatus::OK();
	TestEqual(TEXT("Default OK message"), Status.GetMessage(), TEXT("ok"));
	TestEqual(TEXT("Status type is OK"), Status.GetStatusType(), EExecStatusType::OK);
	TestTrue(TEXT("Operator== OK"), Status == EExecStatusType::OK);
	TestTrue(TEXT("Operator!= Error"), Status != EExecStatusType::Error);

	// OK with a custom message.
	const FExecStatus StatusMsg = FExecStatus::OK(TEXT("hello world"));
	TestEqual(TEXT("Custom OK message"), StatusMsg.GetMessage(), TEXT("hello world"));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExecStatusErrorTest,
	"UnrealCV.Server.ExecStatus.Error",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusErrorTest::RunTest(const FString& Parameters)
{
	const FExecStatus Status = FExecStatus::Error(TEXT("something failed"));
	TestEqual(TEXT("Error message prefix"), Status.GetMessage(), TEXT("error something failed"));
	TestEqual(TEXT("Status type is Error"), Status.GetStatusType(), EExecStatusType::Error);
	TestTrue(TEXT("Operator== Error"), Status == EExecStatusType::Error);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExecStatusBinaryTest,
	"UnrealCV.Server.ExecStatus.Binary",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusBinaryTest::RunTest(const FString& Parameters)
{
	TArray<uint8> TestData;
	TestData.Add(0x01);
	TestData.Add(0x02);
	TestData.Add(0x03);

	const FExecStatus Status = FExecStatus::Binary(TestData);
	TestEqual(TEXT("Binary data preserved"), Status.GetData(), TestData);
	TestEqual(TEXT("Binary status is OK"), Status.GetStatusType(), EExecStatusType::OK);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExecStatusConcatTest,
	"UnrealCV.Server.ExecStatus.Concatenation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusConcatTest::RunTest(const FString& Parameters)
{
	FExecStatus A = FExecStatus::OK(TEXT("first"));
	const FExecStatus B = FExecStatus::OK(TEXT("second"));

	A += B;
	TestTrue(TEXT("Concatenated contains first"), A.GetMessage().Contains(TEXT("first")));
	TestTrue(TEXT("Concatenated contains second"), A.GetMessage().Contains(TEXT("second")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExecStatusBinaryArrayRoundtripTest,
	"UnrealCV.Server.ExecStatus.BinaryArrayRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusBinaryArrayRoundtripTest::RunTest(const FString& Parameters)
{
	const FString Original = TEXT("Hello UnrealCV");

	TArray<uint8> BinaryData;
	FExecStatus::BinaryArrayFromString(Original, BinaryData);
	TestTrue(TEXT("Binary array is not empty"), BinaryData.Num() > 0);

	// GetData on OK with empty binary should produce binary from message
	const FExecStatus Status = FExecStatus::OK(Original);
	const TArray<uint8> DataOut = Status.GetData();
	TestTrue(TEXT("GetData produces non-empty output"), DataOut.Num() > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExecStatusSentinelsTest,
	"UnrealCV.Server.ExecStatus.Sentinels",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusSentinelsTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("InvalidArgument is Error"), FExecStatus::InvalidArgument == EExecStatusType::Error);
	TestTrue(TEXT("NotImplemented is Error"),  FExecStatus::NotImplemented  == EExecStatusType::Error);
	TestTrue(TEXT("InvalidPointer is Error"),  FExecStatus::InvalidPointer  == EExecStatusType::Error);

	return true;
}

#endif // WITH_AUTOMATION_WORKER
