#pragma once

class FExecStatus;
DECLARE_DELEGATE_RetVal(FExecStatus, FPromiseDelegate); // Check task status

/**
 * Return by async task, used to check status to see whether the task is finished.
 */
class FPromise
{
private:
	/** The method to check whether this promise is alreay completed */
	FPromiseDelegate PromiseDelegate;
public:
	bool bIsValid;
	FDateTime InitTime;
	FPromise() { bIsValid = false; }
	FPromise(FPromiseDelegate InPromiseDelegate) : PromiseDelegate(InPromiseDelegate)
	{
		bIsValid = true;
		InitTime = FDateTime::Now();
	}
	/** Use PromiseDelegate to check whether the task already completed */
	FExecStatus CheckStatus();
	float GetRunningTime()
	{
		FTimespan Elapsed = FDateTime::Now() - InitTime;
		return Elapsed.GetTotalSeconds();
	}
};

enum FExecStatusType
{
	OK,
	Error,
	// Support async task
	Pending,
	AsyncQuery,
};

/**
 * Present the return value of a command. If the FExecStatusType is pending, check the promise value.
 */
class UNREALCV_API FExecStatus
{
public:
	// Define some typical FExecStatus for convinience
	/** OK : Message */
	static FExecStatus OK(FString Message="");
	/** Error : ErrorMessage */
	static FExecStatus Error(FString ErrorMessage);
	/** Error : Argument Invalid */
	static FExecStatus InvalidArgument;
	/** Pending : Message */
	static FExecStatus Pending(FString Message=""); // Useful for async task
	/** Binary : A binary array */
	static FExecStatus Binary(TArray<uint8>& InBinaryData);

	/**
	 * For an async task, return a special pending FExecStatus
	 * Use the CheckStatus of FPromise to check whether the task is completed
	 * If the CheckStatus function returns no longer pending, means the async task finished
	 * see UE4CVCommandsCamera.cpp : GetCameraViewAsyncQuery for an example
	 */
	static FExecStatus AsyncQuery(FPromise Promise);

	/** The message body of this ExecStatus, the full message will also include the ExecStatusType */
	FString MessageBody;
	/** An enum type to show the ExecStatus */
	FExecStatusType ExecStatusType;

	~FExecStatus();
	/** Convert this ExecStatus to String */
	FString GetMessage() const;

	/** Convert this ExecStatus to a binary array */
	TArray<uint8> GetData() const;

	/** Add this FExecStatus with other FExecStatus, useful for executing a few commands at the same time */
	FExecStatus& operator+=(const FExecStatus& InExecStatus);

	/** Return the promise of this FExecStatus, will only be set if the status is pending */
	FPromise& GetPromise();

	/** Convert string to binary array */
	static void BinaryArrayFromString(const FString& Message, TArray<uint8>& OutBinaryArray);
private:
	/** The promise to check result, only useful for async tasks */
	FPromise Promise;
	FExecStatus(FExecStatusType InExecStatusType, FString Message);
	// For query
	FExecStatus(FExecStatusType InExecStatusType, FPromise Promise);
	/** Construct from binary data */
	FExecStatus(FExecStatusType InExecStatusType, TArray<uint8>& InBinaryData);
	/** Binary data */
	TArray<uint8> BinaryData;
};

bool operator==(const FExecStatus& ExecStatus, const FExecStatusType& ExecStatusEnum);
bool operator!=(const FExecStatus& ExecStatus, const FExecStatusType& ExecStatusEnum);
