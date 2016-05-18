// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Regex.h"


/**
 * 
 */
class FExecStatus;
class FPromise;

DECLARE_DELEGATE_RetVal(FExecStatus, FPromiseDelegate);
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
	Query,
	Callback
};

DECLARE_DELEGATE(FCallbackDelegate);
class FExecStatus
{
public:
	// Support aync task
	FPromise Promise; 
	static FExecStatus Pending(FString Message=""); // Useful for async task
	static FExecStatus AsyncQuery(FPromise Promise);
	static FExecStatus AsyncCallback();

	// The status of the FExecStatus can be changed during the async task
	FString MessageBody;
	FExecStatusType ExecStatusType;

	static FExecStatus OK(FString Message="");
	static FExecStatus InvalidArgument;
	static FExecStatus Error(FString ErrorMessage);
	~FExecStatus();
	FString GetMessage() const; // Convert this ExecStatus to String
	FCallbackDelegate& OnFinished();
private:
	FExecStatus(FExecStatusType InExecStatusType, FString Message);
	// For query
	FExecStatus(FExecStatusType InExecStatusType, FPromise Promise);
	// FExecStatus& operator+=(const FExecStatus& InExecStatus);
	FCallbackDelegate FinishedDelegate;
};

bool operator==(const FExecStatus& ExecStatus, const FExecStatusType& ExecStatusEnum);


DECLARE_DELEGATE_RetVal_OneParam(FExecStatus, FDispatcherDelegate, const TArray< FString >&);
class REALISTICRENDERING_API FCommandDispatcher
{
public:
	FCommandDispatcher();
	~FCommandDispatcher();
	bool BindCommand(const FString& UriTemplate, const FDispatcherDelegate& Command, const FString& Description); // Parse URI
	bool Alias(const FString& Alias, const FString& Command, const FString& Description);
	bool Alias(const FString& Alias, const TArray<FString>& Commands, const FString& Description);
	// bool BindCommand(const FString Uri, const FConsoleCommandDelegate& Command); // Parse URI
	FExecStatus Exec(const FString Uri);
	FExecStatus AliasHelper(const TArray<FString>& Args);
	const TMap<FString, FString>& GetUriDescription();

private:
	TMap<FString, FDispatcherDelegate> UriMapping;
	TMap<FString, FString> UriDescription; // Contains help message 
	TMap<FString, TArray<FString> > AliasMapping;
	TMap<FString, FString> TypeRegexp; // Define regular expression for type
	uint32 NumArgsLimit = 32;

	bool FormatUri(const FString& RawUri, FString& UriRexexp);
};
