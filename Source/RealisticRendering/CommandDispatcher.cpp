// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "Regex.h"
#include "CommandDispatcher.h"

FCommandDispatcher::FCommandDispatcher()
{
}

FCommandDispatcher::~FCommandDispatcher()
{
}

bool FCommandDispatcher::BindCommand(const FString UriTemplate, const FConsoleCommandWithArgsDelegate& Command) // Parse URI
{
	// TODO: Build a tree structure to boost performance
	if (UriMapping.Contains(UriTemplate))
	{
		UE_LOG(LogTemp, Warning, TEXT("The UriTemplate %s already exist, overwrited."), *UriTemplate);
	}
	UriMapping.Emplace(UriTemplate, Command);
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


bool FCommandDispatcher::Exec(const FString Uri)
{
	TArray<FString> Args; // Get args from URI
	for (auto& Elem : UriMapping)
	{
		FRegexPattern Pattern = FRegexPattern(Elem.Key);
		FConsoleCommandWithArgsDelegate& Cmd = Elem.Value;
		
		FRegexMatcher Matcher(Pattern, Uri);
		if (Matcher.FindNext())
		{
			// for (uint32 GroupIndex = 1)
			for (uint32 GroupIndex = 1; GroupIndex < NumArgsLimit + 1; GroupIndex++)
			{ 
				uint32 BeginIndex = Matcher.GetCaptureGroupBeginning(GroupIndex);
				if (BeginIndex == -1) break;
				FString Match = Matcher.GetCaptureGroup(GroupIndex);
				Args.Add(Match);
				break;
			}
			Cmd.Execute(Args);
			break;
		}
		
		// TODO: Regular expression mapping is slow, need to implement in a more efficient way.
		// FRegexMatcher()
		
	}
	return true;
}
