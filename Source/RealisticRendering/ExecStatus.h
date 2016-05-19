#pragma once

class FExecStatus;
DECLARE_DELEGATE_RetVal(FExecStatus, FPromiseDelegate); // Check task status

class FPromise 
// Return by async task, used to check status to see whether the task is finished.
{
private:
	FPromiseDelegate PromiseDelegate;
public:
	FPromise();
	FPromise(FPromiseDelegate InPromiseDelegate);
	FExecStatus CheckStatus();
};

enum FExecStatusType 
{
	OK,
	Error,
	// Support async task
	Pending,
};

class FExecStatus
{
public:
	// Support aync task
	FPromise Promise; 
	static FExecStatus Pending(FString Message=""); // Useful for async task
	static FExecStatus AsyncQuery(FPromise Promise);

	// The status of the FExecStatus can be changed during the async task
	FString MessageBody;
	FExecStatusType ExecStatusType;

	static FExecStatus OK(FString Message="");
	static FExecStatus InvalidArgument;
	static FExecStatus Error(FString ErrorMessage);
	~FExecStatus();
	FString GetMessage() const; // Convert this ExecStatus to String
private:
	FExecStatus(FExecStatusType InExecStatusType, FString Message);
	// For query
	FExecStatus(FExecStatusType InExecStatusType, FPromise Promise);
	// FExecStatus& operator+=(const FExecStatus& InExecStatus);
};

bool operator==(const FExecStatus& ExecStatus, const FExecStatusType& ExecStatusEnum);
bool operator!=(const FExecStatus& ExecStatus, const FExecStatusType& ExecStatusEnum);
