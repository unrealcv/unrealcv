// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

class FExecStatus;
DECLARE_DELEGATE_RetVal(FExecStatus, FPromiseDelegate);

/**
 * Returned by async tasks. Call CheckStatus() to poll whether the task has completed.
 */
class UNREALCV_API FPromise
{
public:
	bool bIsValid = false;
	FDateTime InitTime;

	FPromise() = default;

	explicit FPromise(FPromiseDelegate InPromiseDelegate)
		: bIsValid(true)
		, InitTime(FDateTime::Now())
		, PromiseDelegate(MoveTemp(InPromiseDelegate))
	{
	}

	FExecStatus CheckStatus();

	float GetRunningTime() const
	{
		const FTimespan Elapsed = FDateTime::Now() - InitTime;
		return static_cast<float>(Elapsed.GetTotalSeconds());
	}

private:
	FPromiseDelegate PromiseDelegate;
};

/**
 * Strongly-typed status codes for command execution results.
 */
enum class EExecStatusType : uint8
{
	OK,
	Error,
};

/**
 * Represents the return value of a command execution.
 */
class UNREALCV_API FExecStatus
{
public:
	static FExecStatus OK(const FString& Message = TEXT(""));
	static FExecStatus Error(const FString& ErrorMessage);
	static FExecStatus Binary(TArray<uint8>& InBinaryData);

	static FExecStatus InvalidArgument;
	static FExecStatus NotImplemented;
	static FExecStatus InvalidPointer;

	FString GetMessage() const;
	TArray<uint8> GetData() const;
	FPromise& GetPromise() { return Promise; }

	FExecStatus& operator+=(const FExecStatus& InExecStatus);

	static void BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray);

	~FExecStatus() = default;

	FString MessageBody;
	EExecStatusType ExecStatusType = EExecStatusType::OK;

private:
	FExecStatus(EExecStatusType InExecStatusType, const FString& Message);
	FExecStatus(EExecStatusType InExecStatusType, FPromise InPromise);
	FExecStatus(EExecStatusType InExecStatusType, TArray<uint8>& InBinaryData);

	FPromise Promise;
	TArray<uint8> BinaryData;
};

bool operator==(const FExecStatus& ExecStatus, EExecStatusType ExecStatusEnum);
bool operator!=(const FExecStatus& ExecStatus, EExecStatusType ExecStatusEnum);
