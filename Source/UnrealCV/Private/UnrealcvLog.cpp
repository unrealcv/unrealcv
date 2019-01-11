// Weichao Qiu @ 2018
#include "UnrealcvLog.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

void FUnrealcvLogger::ScreenLog(const FString& Message)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
}

void FUnrealcvLogger::LogOnce(const FString& LogMessage)
{
	if (ErrorList.Contains(LogMessage)) return;
	
	// Log warning message
	UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *LogMessage);
	ErrorList.Add(LogMessage);
}

