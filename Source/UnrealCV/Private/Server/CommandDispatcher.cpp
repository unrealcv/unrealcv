// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "CommandDispatcher.h"
#include "UnrealcvStats.h"
#include "UnrealcvLog.h"

DECLARE_CYCLE_STAT(TEXT("FCommandDispatcher::Exec"), STAT_Exec, STATGROUP_UnrealCV);

FCommandDispatcher::FCommandDispatcher()
{
	const FString Str   = TEXT("([^ ]*)");
	const FString UInt  = TEXT("(\\d*)");
	const FString Float = TEXT("([-+]?\\d*[.]?\\d+)");

	TypeRegexp.Emplace(TEXT("str"),   Str);
	TypeRegexp.Emplace(TEXT("uint"),  UInt);
	TypeRegexp.Emplace(TEXT("float"), Float);

	FDispatcherDelegate Cmd = FDispatcherDelegate::CreateRaw(this, &FCommandDispatcher::AliasHelper);
	BindCommand(TEXT("vrun [str]"), Cmd, TEXT("Run an alias for UnrealCV plugin"));
}

bool FCommandDispatcher::FormatUri(const FString& RawUri, FString& OutRegex) const
{
	FString Result;
	bool bInsideSpecifier = false;
	FString TypeSpecifier;

	for (int32 Index = 0; Index < RawUri.Len(); ++Index)
	{
		const TCHAR Ch = RawUri[Index];

		if (!bInsideSpecifier)
		{
			if (Ch == TEXT('[')) { bInsideSpecifier = true; continue; }
			if (Ch == TEXT(']'))
			{
				UE_LOG(LogUnrealCV, Error, TEXT("Unexpected ']' at position %d in URI template"), Index);
				return false;
			}
			Result += Ch;
			continue;
		}

		if (Ch == TEXT('['))
		{
			UE_LOG(LogUnrealCV, Error, TEXT("Unexpected '[' at position %d in URI template"), Index);
			return false;
		}
		if (Ch == TEXT(']'))
		{
			bInsideSpecifier = false;
			if (const FString* RegexFragment = TypeRegexp.Find(TypeSpecifier))
			{
				Result += *RegexFragment;
				TypeSpecifier.Empty();
				continue;
			}
			UE_LOG(LogUnrealCV, Error, TEXT("Unknown type specifier '%s'"), *TypeSpecifier);
			return false;
		}
		TypeSpecifier += Ch;
	}

	if (!TypeSpecifier.IsEmpty())
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Unclosed '[' in URI template"));
		return false;
	}

	OutRegex = Result + TEXT("[ ]*$");
	return true;
}

bool FCommandDispatcher::BindCommand(const FString& ReadableUriTemplate, const FDispatcherDelegate& Command, const FString& Description)
{
	FString UriRegex;
	if (!FormatUri(ReadableUriTemplate, UriRegex))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Malformed URI template: %s"), *ReadableUriTemplate);
		return false;
	}

	if (UriMapping.Contains(UriRegex))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("URI '%s' already registered — overwriting."), *UriRegex);
		UriMapping.Remove(UriRegex);
	}

	UriMapping.Emplace(UriRegex, Command);
	UriDescription.Emplace(ReadableUriTemplate, Description);
	UriRegexPattern.Emplace(UriRegex, FRegexPattern(UriRegex));
	UriList.AddUnique(UriRegex);
	return true;
}

bool FCommandDispatcher::Alias(const FString& InAlias, const TArray<FString>& Commands, const FString& Description)
{
	if (AliasMapping.Contains(InAlias))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Alias '%s' already exists — overwriting."), *InAlias);
	}
	AliasMapping.Emplace(InAlias, Commands);
	return true;
}

bool FCommandDispatcher::Alias(const FString& InAlias, const FString& Command, const FString& Description)
{
	TArray<FString> Commands;
	Commands.Add(Command);
	return Alias(InAlias, Commands, Description);
}

FExecStatus FCommandDispatcher::AliasHelper(const TArray<FString>& Args)
{
	if (Args.Num() != 1)
	{
		return FExecStatus::Error(TEXT("Alias does not support extra parameters"));
	}

	const FString& AliasName = Args[0];
	const TArray<FString>* Commands = AliasMapping.Find(AliasName);
	if (!Commands)
	{
		return FExecStatus::Error(FString::Printf(TEXT("Unrecognised alias '%s'"), *AliasName));
	}

	FString CombinedMessage;
	for (const FString& Command : *Commands)
	{
		const FExecStatus Status = Exec(Command);
		if (!CombinedMessage.IsEmpty()) { CombinedMessage += TEXT("\n"); }
		CombinedMessage += Status.GetMessage();
	}
	return FExecStatus::OK(CombinedMessage);
}

const TMap<FString, FString>& FCommandDispatcher::GetUriDescription() const
{
	return UriDescription;
}

FExecStatus FCommandDispatcher::Exec(const FString& Uri)
{
	SCOPE_CYCLE_COUNTER(STAT_Exec);

	if (!IsInGameThread())
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Command execution must happen on the game thread."));
		return FExecStatus::Error(TEXT("Command execution must happen on the game thread."));
	}

	for (int32 UriIndex = UriList.Num() - 1; UriIndex >= 0; --UriIndex)
	{
		const FString& Key = UriList[UriIndex];
		const FRegexPattern& Pattern = UriRegexPattern[Key];
		FRegexMatcher Matcher(Pattern, Uri);

		if (!Matcher.FindNext()) { continue; }

		TArray<FString> Args;
		for (uint32 GroupIndex = 1; GroupIndex <= NumArgsLimit; ++GroupIndex)
		{
			const int32 BeginIndex = Matcher.GetCaptureGroupBeginning(GroupIndex);
			if (BeginIndex == INDEX_NONE) { break; }
			Args.Add(Matcher.GetCaptureGroup(GroupIndex));
		}

		FDispatcherDelegate& Cmd = UriMapping[Key];
		if (Cmd.IsBound()) { return Cmd.Execute(Args); }

		const FString ErrorMsg = TEXT("Command delegate is not bound.");
		UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *ErrorMsg);
		return FExecStatus::Error(ErrorMsg);
	}

	return FExecStatus::Error(FString::Printf(TEXT("No handler found for URI '%s'"), *Uri));
}
