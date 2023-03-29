// Weichao Qiu @ 2016
#include "CommandDispatcher.h"
#include "Runtime/Core/Public/HAL/Runnable.h"
#include "Runtime/Core/Public/HAL/RunnableThread.h"
#include "Runtime/Core/Public/Async/Async.h"
#include "Runtime/Core/Public/Containers/Queue.h"
#include "Runtime/Core/Public/Internationalization/Regex.h"
#include "UnrealcvStats.h"
#include "UnrealcvLog.h"

// CommandDispatcher->BindCommand("vset /mode/(?<ViewMode>.*)", SetViewMode); // Better to check the correctness at compile time
// The regular expression for float number is from here, http://stackoverflow.com/questions/12643009/regular-expression-for-floating-point-numbers
// Use ICU regexp to define URI, See http://userguide.icu-project.org/strings/regexp

DECLARE_CYCLE_STAT(TEXT("FCommandDispatcher::Exec"), STAT_Exec, STATGROUP_UnrealCV);

FCommandDispatcher::FCommandDispatcher()
{
	FString Str = "([^ ]*)", UInt = "(\\d*)", Float = "([-+]?\\d*[.]?\\d+)"; // Each type will be considered as a group
	TypeRegexp.Emplace("str", Str);
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
				UE_LOG(LogUnrealCV, Error, TEXT("Unexpected ] in %d"), Index);
				// check(false);
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
				UE_LOG(LogUnrealCV, Error, TEXT("Unexpected [ in %d"), Index);
				// check(false);
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
					UE_LOG(LogUnrealCV, Error, TEXT("Unknown type specifier "));
					// check(false);
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
		UE_LOG(LogUnrealCV, Error, TEXT("Not all [ are closed by ]"));
		// check(false);
		return false;
	}
	UriRexexp = Uri + "[ ]*$"; // Make sure no more parameters
	return true;
}

/** Bind command to a function, the command on the bottom will overwrite the command on the top */
bool FCommandDispatcher::BindCommand(const FString& ReadableUriTemplate, const FDispatcherDelegate& Command, const FString& Description) // Parse URI
{
	// TODO: Build a tree structure to boost performance
	// TODO: The order should matter

	FString UriTemplate;
	if (!FormatUri(ReadableUriTemplate, UriTemplate))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("The UriTemplate %s is malformat"), *ReadableUriTemplate);
		// check(false);
		return false;
	}

	if (UriMapping.Contains(UriTemplate))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The UriTemplate %s already exist, overwrited."), *UriTemplate);
	}

	if (UriMapping.Contains(UriTemplate))
	{
		// Remove existing uri and add, in order to overwrite existing commands
		UriMapping.Remove(UriTemplate);
	}
	UriMapping.Emplace(UriTemplate, Command);
	UriDescription.Emplace(ReadableUriTemplate, Description);
	FRegexPattern Pattern = FRegexPattern(UriTemplate);
	UriRegexPattern.Emplace(UriTemplate, Pattern);
	UriList.AddUnique(UriTemplate);
	return true;
}

bool FCommandDispatcher::Alias(const FString& InAlias, const TArray<FString>& Commands, const FString& Description)
{
	if (AliasMapping.Contains(InAlias))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Alias %s already exist, overwrite."), *InAlias);
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
				Message += ExecStatus.GetMessage(); // TODO: Ovewrite the + operator of ExecStatus
			}
			return FExecStatus::OK(Message); // TODO: Check whether one of them failed and set ExecStatus based on that.
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
	SCOPE_CYCLE_COUNTER(STAT_Exec);
	if (!IsInGameThread())
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Command execution is not in the game thread."));
		return FExecStatus::Error("Command execution is not in the game thread.");
	}
	TArray<FString> Args; // Get args from URI

	// The newly added command should overwrite previous one.
	// for (auto& Elem : UriMapping)
	/*UE_LOG(LogUnrealCV, Warning, TEXT("Number of mach list: %d"), UriList.Num());
	UE_LOG(LogUnrealCV, Warning, TEXT("Uri need to be matched : %s"), *Uri);*/

	// Iterate the UriList in the reverse order
	for (int UriIndex = UriList.Num() - 1; UriIndex >= 0; UriIndex--)
	{
		// FRegexPattern Pattern = FRegexPattern(Elem.Key);
		FString Key = UriList[UriIndex];
		//UE_LOG(LogUnrealCV, Warning, TEXT("match list item : %s"), *Key);
		FRegexPattern Pattern = UriRegexPattern[Key];

		FRegexMatcher Matcher(Pattern, Uri);
		if (Matcher.FindNext())
		{
			for (uint32 GroupIndex = 1; GroupIndex < NumArgsLimit + 1; GroupIndex++)
			{
				uint32 BeginIndex = Matcher.GetCaptureGroupBeginning(GroupIndex);
				if (BeginIndex == -1) break; // No more matching group to extract
				FString Match = Matcher.GetCaptureGroup(GroupIndex); // TODO: Strip empty space
				Args.Add(Match);
			}
			FDispatcherDelegate& Cmd = UriMapping[Key];
			if (Cmd.IsBound())
			{
				return Cmd.Execute(Args); // The exec status can be successful, fail or give a message back
			}
			else
			{
				FString ErrorMsg = TEXT("Command delegate is not bound.");
				UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ErrorMsg);
				return FExecStatus::Error(ErrorMsg);
			}
		}

		// TODO: Regular expression mapping is slow, need to implement in a more efficient way.
		// FRegexMatcher()
	}
	return FExecStatus::Error(FString::Printf(TEXT("Can not find a handler for URI '%s'"), *Uri));
}
