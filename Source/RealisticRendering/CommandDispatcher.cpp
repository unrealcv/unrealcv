// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "Regex.h"
#include "CommandDispatcher.h"

// DECLARE_DELEGATE_OneParam(FDispatcherDelegate, const TArray< FString >&);

FExecStatus FExecStatus::OK = FExecStatus("ok"); // The status should start with ok or error for client to understand
FExecStatus FExecStatus::InvalidArgument = FExecStatus("error Argument Invalid");
// FExecStatus FExecStatus::Error = FExecStatus("error");
FExecStatus FExecStatus::Error(FString ErrorMessage)
{
	return FExecStatus(FString::Printf(TEXT("error %s"), *ErrorMessage));
}

FExecStatus::FExecStatus(FString Message)
{
	this->Message = Message;
}

FExecStatus::~FExecStatus()
{
}

FCommandDispatcher::FCommandDispatcher()
{
	// Add Unreal Console Support
	IConsoleObject* VGetCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vget"),
		TEXT("Set resource in Unreal Engine"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FCommandDispatcher::UEConsoleGetHelper)
		);

	IConsoleObject* VSetCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("vset"),
		TEXT("Set resource in Unreal Engine"),
		FConsoleCommandWithArgsDelegate::CreateRaw(this, &FCommandDispatcher::UEConsoleSetHelper)
		);
}

FCommandDispatcher::~FCommandDispatcher()
{
}

void FCommandDispatcher::UEConsoleGetHelper(const TArray<FString>& Args)
{
	// Join string
	FString Cmd = "vget ";
	uint32 NumArgs = Args.Num();
	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; // Maybe a more elegant implementation
	FExecStatus ExecStatus = Exec(Cmd);
	UE_LOG(LogTemp, Warning, TEXT("vget helper function, the real command is %s"), *Cmd);
	// In the console mode, output should be writen to the output log.
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ExecStatus.Message);
}

void FCommandDispatcher::UEConsoleSetHelper(const TArray<FString>& Args)
{
	FString Cmd = "vset ";
	uint32 NumArgs = Args.Num();
	for (uint32 ArgIndex = 0; ArgIndex < NumArgs-1; ArgIndex++)
	{
		Cmd += Args[ArgIndex] + " ";
	}
	Cmd += Args[NumArgs-1]; 
	FExecStatus ExecStatus = Exec(Cmd);
	UE_LOG(LogTemp, Warning, TEXT("vset helper function, the real command is %s"), *Cmd);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *ExecStatus.Message);
}

bool FCommandDispatcher::BindCommand(const FString UriTemplate, const FDispatcherDelegate& Command) // Parse URI
{
	// TODO: Add Unreal Console support
	// TODO: Build a tree structure to boost performance
	if (UriMapping.Contains(UriTemplate))
	{
		UE_LOG(LogTemp, Warning, TEXT("The UriTemplate %s already exist, overwrited."), *UriTemplate);
	}
	UriMapping.Emplace(UriTemplate, Command);
	return true;
}

bool FCommandDispatcher::Alias(const FString Alias, const TArray<FString>& Commands)
{
	// Alias can not support arguments
	return true;
}

bool FCommandDispatcher::Alias(const FString Alias, const FString Command)
{
	// Alias can not support arguments
	return true;
}
	
bool FCommandDispatcher::Match(const FString Uri)
{
	return true;
}


FExecStatus FCommandDispatcher::Exec(const FString Uri)
{
	TArray<FString> Args; // Get args from URI
	for (auto& Elem : UriMapping)
	{
		FRegexPattern Pattern = FRegexPattern(Elem.Key);
		FDispatcherDelegate& Cmd = Elem.Value;
		
		FRegexMatcher Matcher(Pattern, Uri);
		if (Matcher.FindNext())
		{
			// for (uint32 GroupIndex = 1)
			for (uint32 GroupIndex = 1; GroupIndex < NumArgsLimit + 1; GroupIndex++)
			{ 
				uint32 BeginIndex = Matcher.GetCaptureGroupBeginning(GroupIndex);
				if (BeginIndex == -1) break; // No more matching group to extract
				FString Match = Matcher.GetCaptureGroup(GroupIndex); // TODO: Strip empty space
				Args.Add(Match);
			}
			return Cmd.Execute(Args); // The exec status can be successful, fail or give a message back
		}
		
		// TODO: Regular expression mapping is slow, need to implement in a more efficient way.
		// FRegexMatcher()
		
	}
	return FExecStatus::Error(FString::Printf(TEXT("Can not find a handler for URI '%s'"), *Uri));
}
