// Weichao Qiu @ 2016
#include "ExecStatus.h"
#include "Runtime/Launch/Resources/Version.h"
#if ENGINE_MAJOR_VERSION >= 5
#include "Runtime/Core/Public/Containers/StringConv.h"
#endif
// DECLARE_DELEGATE_OneParam(FDispatcherDelegate, const TArray< FString >&);
FExecStatus FExecStatus::InvalidArgument = FExecStatus(EExecStatusType::Error, "Argument Invalid");
FExecStatus FExecStatus::NotImplemented = FExecStatus(EExecStatusType::Error, "Not Implemented");
FExecStatus FExecStatus::InvalidPointer = FExecStatus(EExecStatusType::Error, "Pointer to object invalid, check log for details");

/** Begin of FPromise functions */

FExecStatus FPromise::CheckStatus()
{
	check(this);
	check(PromiseDelegate.IsBound());
	return PromiseDelegate.Execute();
}

/** Begin of FExecStatus functions */
bool operator==(const FExecStatus& ExecStatus, const EExecStatusType& ExecStatusType)
{
	return (ExecStatus.ExecStatusType == ExecStatusType);
}

bool operator!=(const FExecStatus& ExecStatus, const EExecStatusType& ExecStatusType)
{
	return (ExecStatus.ExecStatusType != ExecStatusType);
}

FExecStatus& FExecStatus::operator+=(const FExecStatus& Src)
{
	this->BinaryData += Src.BinaryData;
	this->MessageBody += "\n" + Src.MessageBody;
	return *this;
}


FExecStatus FExecStatus::OK(FString InMessage)
{
	return FExecStatus(EExecStatusType::OK, InMessage);
}


FExecStatus FExecStatus::Error(FString ErrorMessage)
{
	return FExecStatus(EExecStatusType::Error, ErrorMessage);
}

FString FExecStatus::GetMessage() const // Define how to format the reply string
{
	FString TypeName;
	switch (ExecStatusType)
	{
	case EExecStatusType::OK:
		if (MessageBody == "")
			return "ok";
		else
			return MessageBody;
	case EExecStatusType::Error:
		TypeName = "error"; break;
	default:
		TypeName = "unknown FExecStatus Type";
	}
	FString Message = FString::Printf(TEXT("%s %s"), *TypeName, *MessageBody);
	return Message;
}

FExecStatus::FExecStatus(EExecStatusType InExecStatusType, FPromise InPromise)
{
	ExecStatusType = InExecStatusType;
	Promise = InPromise;
}

FExecStatus::FExecStatus(EExecStatusType InExecStatusType, FString InMessage)
{
	ExecStatusType = InExecStatusType;
	MessageBody = InMessage;
}

FExecStatus::~FExecStatus()
{
}

FExecStatus FExecStatus::Binary(TArray<uint8>& BinaryData)
{
	return FExecStatus(EExecStatusType::OK, BinaryData);
}

FExecStatus::FExecStatus(EExecStatusType InExecStatusType, TArray<uint8>& InBinaryData)
{
	ExecStatusType = InExecStatusType;
	BinaryData = InBinaryData;
}

TArray<uint8> FExecStatus::GetData() const // Define how to format the reply string
{
	if (this->BinaryData.Num() != 0)
	{
		return BinaryData;
	}
	FString TypeName;
	FString Message;
	switch (ExecStatusType)
	{
	case EExecStatusType::OK:
		if (MessageBody == "")
			Message = "ok";
		else
			Message = MessageBody;
		break;
	case EExecStatusType::Error:
		TypeName = "error"; break;
	default:
		TypeName = "unknown FExecStatus Type";
	}
	if (ExecStatusType != EExecStatusType::OK)
	{
		Message = FString::Printf(TEXT("%s %s"), *TypeName, *MessageBody);
	}
	TArray<uint8> FormatedBinaryData;
	BinaryArrayFromString(Message, FormatedBinaryData);
	
	return FormatedBinaryData;
}


void FExecStatus::BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray)
{

	//From: https://github.com/EpicGames/UnrealEngine/blob/5.3/Engine/Source/Runtime/Core/Public/Containers/StringConv.h#L339
	/*UE_DEPRECATED(5.1, "FTCHARToUTF8_Convert has been deprecated in favor of FPlatformString::Convert and StringCast")*/
#if ENGINE_MAJOR_VERSION <= 4
	FTCHARToUTF8 Convert(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append((UTF8CHAR*)Convert.Get(), Convert.Length());
#else 
	//https://github.com/EpicGames/UnrealEngine/blob/5.3/Engine/Source/Runtime/Core/Public/Containers/StringConv.hL#L1070
	auto converter = StringCast<UTF8CHAR>(*Message);
	OutBinaryArray.Empty();
	OutBinaryArray.Append((uint8*)converter.Get(), converter.Length());
#endif
}

