// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "Misc/AutomationTest.h"
#include "ExecStatus.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FExecStatusSentinelCopyTest,
    "UnrealCV.Server.ExecStatus.SentinelCopy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusSentinelCopyTest::RunTest(const FString& Parameters)
{
    // Sentinels are const — verify that copies are independent.
    FExecStatus Copy = FExecStatus::InvalidArgument;
    Copy += FExecStatus::OK(TEXT("extra"));

    // The original sentinel must not have been modified.
    TestEqual(TEXT("Sentinel message unchanged"),
        FExecStatus::InvalidArgument.GetMessageBody(), TEXT("Argument Invalid"));
    TestTrue(TEXT("Copy was mutated"),
        Copy.GetMessageBody().Contains(TEXT("extra")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FExecStatusGetDataErrorTest,
    "UnrealCV.Server.ExecStatus.GetDataError",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusGetDataErrorTest::RunTest(const FString& Parameters)
{
    // Verify GetData serialises error status correctly.
    const FExecStatus Err = FExecStatus::Error(TEXT("something broke"));
    const TArray<uint8> Data = Err.GetData();
    const FString Str = FString(UTF8_TO_TCHAR(
        std::string(reinterpret_cast<const char*>(Data.GetData()), Data.Num()).c_str()));
    TestTrue(TEXT("Serialised error contains 'error'"), Str.Contains(TEXT("error")));
    TestTrue(TEXT("Serialised error contains message"), Str.Contains(TEXT("something broke")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FExecStatusGetDataOKTest,
    "UnrealCV.Server.ExecStatus.GetDataOK",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FExecStatusGetDataOKTest::RunTest(const FString& Parameters)
{
    // Empty OK -> "ok"
    const FExecStatus OkEmpty = FExecStatus::OK();
    const TArray<uint8> Data = OkEmpty.GetData();
    const FString Str = FString(UTF8_TO_TCHAR(
        std::string(reinterpret_cast<const char*>(Data.GetData()), Data.Num()).c_str()));
    TestEqual(TEXT("Empty OK serialises as 'ok'"), Str, TEXT("ok"));

    // OK with message -> the message itself
    const FExecStatus OkMsg = FExecStatus::OK(TEXT("hello"));
    const TArray<uint8> Data2 = OkMsg.GetData();
    const FString Str2 = FString(UTF8_TO_TCHAR(
        std::string(reinterpret_cast<const char*>(Data2.GetData()), Data2.Num()).c_str()));
    TestEqual(TEXT("OK with message serialises correctly"), Str2, TEXT("hello"));
    return true;
}
