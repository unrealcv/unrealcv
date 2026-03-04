// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "ExecStatus.h"
#include "SocketUtils.h"

const FExecStatus FExecStatus::InvalidArgument = FExecStatus(EExecStatusType::Error, TEXT("Argument Invalid"));
const FExecStatus FExecStatus::NotImplemented  = FExecStatus(EExecStatusType::Error, TEXT("Not Implemented"));
const FExecStatus FExecStatus::InvalidPointer  = FExecStatus(EExecStatusType::Error, TEXT("Pointer to object invalid, check log for details"));

// ---------------------------------------------------------------------------
// FPromise
// ---------------------------------------------------------------------------

FExecStatus FPromise::CheckStatus()
{
	checkf(PromiseDelegate.IsBound(), TEXT("Promise delegate is not bound"));
	return PromiseDelegate.Execute();
}

// ---------------------------------------------------------------------------
// Comparison operators
// ---------------------------------------------------------------------------

bool operator==(const FExecStatus& ExecStatus, EExecStatusType ExecStatusType)
{
	return ExecStatus.GetStatusType() == ExecStatusType;
}

bool operator!=(const FExecStatus& ExecStatus, EExecStatusType ExecStatusType)
{
	return ExecStatus.GetStatusType() != ExecStatusType;
}

// ---------------------------------------------------------------------------
// Compound assignment
// ---------------------------------------------------------------------------

FExecStatus& FExecStatus::operator+=(const FExecStatus& Src)
{
	BinaryData += Src.BinaryData;
	if (!MessageBody.IsEmpty())
	{
		MessageBody += TEXT("\n");
	}
	MessageBody += Src.MessageBody;
	return *this;
}

// ---------------------------------------------------------------------------
// Factory methods
// ---------------------------------------------------------------------------

FExecStatus FExecStatus::OK(const FString& InMessage)
{
	return FExecStatus(EExecStatusType::OK, InMessage);
}

FExecStatus FExecStatus::Error(const FString& ErrorMessage)
{
	return FExecStatus(EExecStatusType::Error, ErrorMessage);
}

FExecStatus FExecStatus::Binary(const TArray<uint8>& InBinaryData)
{
	return FExecStatus(EExecStatusType::OK, InBinaryData);
}

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

FExecStatus::FExecStatus(EExecStatusType InExecStatusType, const FString& InMessage)
	: MessageBody(InMessage)
	, ExecStatusType(InExecStatusType)
{
}

FExecStatus::FExecStatus(EExecStatusType InExecStatusType, FPromise InPromise)
	: ExecStatusType(InExecStatusType)
	, Promise(MoveTemp(InPromise))
{
}

FExecStatus::FExecStatus(EExecStatusType InExecStatusType, const TArray<uint8>& InBinaryData)
	: ExecStatusType(InExecStatusType)
	, BinaryData(InBinaryData)
{
}

// ---------------------------------------------------------------------------
// Serialisation
// ---------------------------------------------------------------------------

namespace
{
/** Format a status + message body into the wire-protocol string. */
FString FormatStatusString(EExecStatusType StatusType, const FString& Body)
{
	switch (StatusType)
	{
	case EExecStatusType::OK:    return Body.IsEmpty() ? TEXT("ok") : Body;
	case EExecStatusType::Error: return FString::Printf(TEXT("error %s"), *Body);
	default:                     return FString::Printf(TEXT("unknown %s"), *Body);
	}
}
} // anonymous namespace

FString FExecStatus::GetMessage() const
{
	return FormatStatusString(ExecStatusType, MessageBody);
}

TArray<uint8> FExecStatus::GetData() const
{
	if (BinaryData.Num() != 0)
	{
		return BinaryData;
	}

	TArray<uint8> FormattedBinaryData;
	BinaryArrayFromString(FormatStatusString(ExecStatusType, MessageBody), FormattedBinaryData);
	return FormattedBinaryData;
}

void FExecStatus::BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	UCV::SocketUtils::BinaryArrayFromString(Message, OutBinaryArray);
}
