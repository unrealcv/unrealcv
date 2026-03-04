// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "ExecStatus.h"

FExecStatus FExecStatus::InvalidArgument = FExecStatus(EExecStatusType::Error, TEXT("Argument Invalid"));
FExecStatus FExecStatus::NotImplemented  = FExecStatus(EExecStatusType::Error, TEXT("Not Implemented"));
FExecStatus FExecStatus::InvalidPointer  = FExecStatus(EExecStatusType::Error, TEXT("Pointer to object invalid, check log for details"));

// ---------------------------------------------------------------------------
// FPromise
// ---------------------------------------------------------------------------

FExecStatus FPromise::CheckStatus()
{
	check(this);
	check(PromiseDelegate.IsBound());
	return PromiseDelegate.Execute();
}

// ---------------------------------------------------------------------------
// Comparison operators
// ---------------------------------------------------------------------------

bool operator==(const FExecStatus& ExecStatus, EExecStatusType ExecStatusType)
{
	return ExecStatus.ExecStatusType == ExecStatusType;
}

bool operator!=(const FExecStatus& ExecStatus, EExecStatusType ExecStatusType)
{
	return ExecStatus.ExecStatusType != ExecStatusType;
}

// ---------------------------------------------------------------------------
// Compound assignment
// ---------------------------------------------------------------------------

FExecStatus& FExecStatus::operator+=(const FExecStatus& Src)
{
	BinaryData += Src.BinaryData;
	MessageBody += TEXT("\n") + Src.MessageBody;
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

FExecStatus FExecStatus::Binary(TArray<uint8>& InBinaryData)
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

FExecStatus::FExecStatus(EExecStatusType InExecStatusType, TArray<uint8>& InBinaryData)
	: ExecStatusType(InExecStatusType)
	, BinaryData(InBinaryData)
{
}

// ---------------------------------------------------------------------------
// Serialisation
// ---------------------------------------------------------------------------

FString FExecStatus::GetMessage() const
{
	switch (ExecStatusType)
	{
	case EExecStatusType::OK:
		return MessageBody.IsEmpty() ? TEXT("ok") : MessageBody;
	case EExecStatusType::Error:
		return FString::Printf(TEXT("error %s"), *MessageBody);
	default:
		return FString::Printf(TEXT("unknown %s"), *MessageBody);
	}
}

TArray<uint8> FExecStatus::GetData() const
{
	if (BinaryData.Num() != 0)
	{
		return BinaryData;
	}

	FString Message;
	switch (ExecStatusType)
	{
	case EExecStatusType::OK:
		Message = MessageBody.IsEmpty() ? TEXT("ok") : MessageBody;
		break;
	case EExecStatusType::Error:
		Message = FString::Printf(TEXT("error %s"), *MessageBody);
		break;
	default:
		Message = FString::Printf(TEXT("unknown %s"), *MessageBody);
		break;
	}

	TArray<uint8> FormattedBinaryData;
	BinaryArrayFromString(Message, FormattedBinaryData);
	return FormattedBinaryData;
}

void FExecStatus::BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray)
{
	auto Converter = StringCast<UTF8CHAR>(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append(reinterpret_cast<const uint8*>(Converter.Get()), Converter.Length());
}
