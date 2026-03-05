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
	FDateTime InitTime = FDateTime::MinValue();

	FPromise() = default;

	explicit FPromise(FPromiseDelegate InPromiseDelegate)
		: bIsValid(true)
		, InitTime(FDateTime::Now())
		, PromiseDelegate(MoveTemp(InPromiseDelegate))
	{
	}

	[[nodiscard]] FExecStatus CheckStatus();

	[[nodiscard]] float GetRunningTime() const
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
	// -- Factory methods (prefer these over raw construction) -----------------
	[[nodiscard]] static FExecStatus OK(const FString& Message = TEXT(""));
	[[nodiscard]] static FExecStatus Error(const FString& ErrorMessage);
	[[nodiscard]] static FExecStatus Binary(const TArray<uint8>& InBinaryData);

	// -- Sentinel instances (const to prevent accidental mutation) ------------
	static const FExecStatus InvalidArgument;
	static const FExecStatus NotImplemented;
	static const FExecStatus InvalidPointer;

	// -- Accessors ------------------------------------------------------------
	[[nodiscard]] FString GetMessage() const;
	[[nodiscard]] TArray<uint8> GetData() const;
	void AppendDataTo(TArray<uint8>& OutData) const;
	[[nodiscard]] FPromise& GetPromise() { return Promise; }
	[[nodiscard]] const FPromise& GetPromise() const { return Promise; }

	[[nodiscard]] EExecStatusType GetStatusType() const { return ExecStatusType; }
	[[nodiscard]] const FString& GetMessageBody() const { return MessageBody; }

	FExecStatus& operator+=(const FExecStatus& InExecStatus);

	static void BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray);

	~FExecStatus() = default;

private:
	FExecStatus() = default;
	FExecStatus(EExecStatusType InExecStatusType, const FString& Message);
	FExecStatus(EExecStatusType InExecStatusType, FPromise InPromise);
	FExecStatus(EExecStatusType InExecStatusType, const TArray<uint8>& InBinaryData);

	FString MessageBody;
	EExecStatusType ExecStatusType = EExecStatusType::OK;
	FPromise Promise;
	TArray<uint8> BinaryData;
};

[[nodiscard]] bool operator==(const FExecStatus& ExecStatus, EExecStatusType ExecStatusEnum);
[[nodiscard]] bool operator!=(const FExecStatus& ExecStatus, EExecStatusType ExecStatusEnum);
