// Copyright (c) 2016-2024, UnrealCV Contributors. All Rights Reserved.
#include "UnrealcvLog.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogUnrealCV);

void FUnrealcvLogger::ScreenLog(const FString& Message)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
	}
}

void FUnrealcvLogger::LogOnce(const FString& LogMessage)
{
	if (SeenMessages.Contains(LogMessage))
	{
		return;
	}
	SeenMessages.Add(LogMessage);
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *LogMessage);
}
