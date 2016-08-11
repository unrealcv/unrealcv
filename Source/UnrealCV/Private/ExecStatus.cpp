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
	bIsValid = false;
}

FPromise::FPromise(FPromiseDelegate InPromiseDelegate) : PromiseDelegate(InPromiseDelegate) 
{
	bIsValid = true;
}

FExecStatus FPromise::CheckStatus()
{
	check(this);
	check(PromiseDelegate.IsBound());
	return PromiseDelegate.Execute();
}

// DECLARE_DELEGATE_OneParam(FDispatcherDelegate, const TArray< FString >&);
FExecStatus FExecStatus::InvalidArgument = FExecStatus(FExecStatusType::Error, "Argument Invalid");

FPromise& FExecStatus::GetPromise()
{ 
	check(this->ExecStatusType == FExecStatusType::AsyncQuery); 
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
	return FExecStatus(FExecStatusType::AsyncQuery, Promise);
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
	case FExecStatusType::AsyncQuery:
		TypeName = "async"; break;
	case FExecStatusType::Pending:
		TypeName = "pending"; break;
	default:
		TypeName = "unknown FExecStatus Type";
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
}

FExecStatus::~FExecStatus()
{
}
