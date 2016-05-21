// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "ExecStatus.h"

bool operator==(const FExecStatus& ExecStatus, const FExecStatusType& ExecStatusType)
{
	return (ExecStatus.ExecStatusType == ExecStatusType);
}

bool operator!=(const FExecStatus& ExecStatus, const FExecStatusType& ExecStatusType)
{
	return (ExecStatus.ExecStatusType != ExecStatusType);
}

FExecStatus& FExecStatus::operator+=(const FExecStatus& Src)
{
	this->MessageBody += "\n" + Src.MessageBody;
	if (Src.ExecStatusType == FExecStatusType::OK)
	{
		
	}
	return *this;
}

FPromise::FPromise()
{
}

FPromise::FPromise(FPromiseDelegate InPromiseDelegate) : PromiseDelegate(InPromiseDelegate) {}

FExecStatus FPromise::CheckStatus()
{
	check(this);
	return PromiseDelegate.Execute();
}

// DECLARE_DELEGATE_OneParam(FDispatcherDelegate, const TArray< FString >&);
// FExecStatus FExecStatus::Pending = FExecStatus("pending");
// FExecStatus FExecStatus::OK = FExecStatus("ok"); // The status should start with ok or error for client to understand
FExecStatus FExecStatus::InvalidArgument = FExecStatus(FExecStatusType::Error, "Argument Invalid");
// FExecStatus FExecStatus::Error = FExecStatus("error");

FPromise& FExecStatus::GetPromise()
{ 
	check(this->ExecStatusType == FExecStatusType::Pending); 
	// Check should be in a single line, make it easier to read and debug
	// The promise is set only when the operation is async and ExecStatus is pending
	return this->Promise; 
}
	
FExecStatus FExecStatus::OK(FString InMessage)
{
	return FExecStatus(FExecStatusType::OK, InMessage);
}

FExecStatus FExecStatus::Pending(FString InMessage)
{
	return FExecStatus(FExecStatusType::Pending, InMessage);
}

FExecStatus FExecStatus::AsyncQuery(FPromise Promise)
{
	return FExecStatus(FExecStatusType::Pending, Promise);
}

FExecStatus FExecStatus::Error(FString ErrorMessage)
{
	return FExecStatus(FExecStatusType::Error, ErrorMessage);
}

FString FExecStatus::GetMessage() const // Define how to format the reply string
{
	FString TypeName;
	switch (ExecStatusType)
	{
	case FExecStatusType::OK:
		if (MessageBody == "")
			return "ok";
		else
			return MessageBody;
	case FExecStatusType::Error: 
		TypeName = "error"; break;
	case FExecStatusType::Pending:
		TypeName = "pending"; break;
	default:
		TypeName = "unknown";
	}
	FString Message = FString::Printf(TEXT("%s %s"), *TypeName, *MessageBody);
	return Message;
}

FExecStatus::FExecStatus(FExecStatusType InExecStatusType, FPromise InPromise)
{
	ExecStatusType = InExecStatusType;
	Promise = InPromise;
}

FExecStatus::FExecStatus(FExecStatusType InExecStatusType, FString InMessage)
{
	ExecStatusType = InExecStatusType;
	MessageBody = InMessage;
	/*
	if (InMessage != "")
	{ 
		Message = InMessage;
	}
	else
	{
		Message = "error Message can not be empty";
	}
	*/
}

FExecStatus::~FExecStatus()
{
}
