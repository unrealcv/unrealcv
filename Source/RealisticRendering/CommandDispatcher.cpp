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
	if (Message != "")
	{ 
		this->Message = Message;
	}
	else
	{
		this->Message = "error Message can not be empty";
	}
}

FExecStatus::~FExecStatus()
{
}

FCommandDispatcher::FCommandDispatcher()
{
	FString Any = "(.*)", UInt = "(\\d*)", Float = "([-+]?\\d*[.]?\\d+)"; // Each type will be considered as a group
	TypeRegexp.Emplace("str", Any);
	TypeRegexp.Emplace("uint", UInt);
	TypeRegexp.Emplace("float", Float);

	FDispatcherDelegate Cmd = FDispatcherDelegate::CreateRaw(this, &FCommandDispatcher::AliasHelper);
	FString Uri = FString::Printf(TEXT("vrun [str]"));
	BindCommand(Uri, Cmd, "Run an alias for Unreal CV plugin");
}

FCommandDispatcher::~FCommandDispatcher()
{
}

bool FCommandDispatcher::FormatUri(const FString& RawUri, FString& UriRexexp)
{
	FString Uri = "";
	bool IsTypeSpecifier = false;
	FString TypeSpecifier = "";

	// Implement a simple state-machine to parse input string
	for (int32 Index = 0; Index < RawUri.Len(); Index++)
	{
		TCHAR Ch = RawUri[Index];
		if (!IsTypeSpecifier)
		{
			if (Ch == '[')
			{
				IsTypeSpecifier = true;
				continue;
			}
			if (Ch == ']')
			{
				UE_LOG(LogTemp, Error, TEXT("Unexpected ] in %d"), Index);
				return false;
			}
			// else
			Uri += Ch;
			continue;
		}
		if (IsTypeSpecifier)
		{
			if (Ch == '[')
			{
				UE_LOG(LogTemp, Error, TEXT("Unexpected [ in %d"), Index);
				return false;
			}
			if (Ch == ']')
			{
				IsTypeSpecifier = false;
				if (TypeRegexp.Contains(TypeSpecifier))
				{
					Uri += TypeRegexp[TypeSpecifier];
					TypeSpecifier = "";
					continue;
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Unknown type specifier "));
					return false;
				}
			}
			// else
			TypeSpecifier += Ch;
			continue;
		}
	}

	if (TypeSpecifier != "")
	{
		UE_LOG(LogTemp, Error, TEXT("Not all [ are closed by ]"));
		return false;
	}
	UriRexexp = Uri;
	return true;
}

bool FCommandDispatcher::BindCommand(const FString& RawUriTemplate, const FDispatcherDelegate& Command, const FString& Description) // Parse URI
{
	// TODO: Build a tree structure to boost performance
	// TODO: The order should matter

	FString UriTemplate;
	if (!FormatUri(RawUriTemplate, UriTemplate))
	{
		UE_LOG(LogTemp, Error, TEXT("The UriTemplate %s is malformat"), *RawUriTemplate);
		check(false);
		return false;
	}

	if (UriMapping.Contains(UriTemplate))
	{
		UE_LOG(LogTemp, Warning, TEXT("The UriTemplate %s already exist, overwrited."), *UriTemplate);
	}
	UriMapping.Emplace(UriTemplate, Command);
	UriDescription.Emplace(RawUriTemplate, Description);
	return true;
}

bool FCommandDispatcher::Alias(const FString& InAlias, const TArray<FString>& Commands, const FString& Description)
{
	if (AliasMapping.Contains(InAlias))
	{
		UE_LOG(LogTemp, Warning, TEXT("Alias %s already exist, overwrite."), *InAlias);
	}
	AliasMapping.Emplace(InAlias, Commands);
	// Alias can not support arguments
	return true;
}

bool FCommandDispatcher::Alias(const FString& InAlias, const FString& Command, const FString& Description)
{
	TArray<FString> Commands;
	Commands.Add(Command);
	Alias(InAlias, Commands, Description);
	// Alias can not support arguments
	return true;
}

FExecStatus FCommandDispatcher::AliasHelper(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString AliasName = Args[0];
		if (AliasMapping.Contains(AliasName))
		{
			TArray<FString>& Commands = AliasMapping[AliasName];
			FString Message = "";
			for (FString Command : Commands)
			{
				FExecStatus ExecStatus = Exec(Command);
				Message += ExecStatus.Message; // TODO: Ovewrite the + operator of ExecStatus
			}
			return FExecStatus(Message); // TODO: Check whether one of them failed and set ExecStatus based on that.
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Unrecognized alias %s"), *AliasName));
		}
	}
	return FExecStatus::Error("Alias can not support extra parameters");
}

const TMap<FString, FString>& FCommandDispatcher::GetUriDescription()
{
	return this->UriDescription;
}

	
FExecStatus FCommandDispatcher::Exec(const FString Uri)
{
	check(IsInGameThread());
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
