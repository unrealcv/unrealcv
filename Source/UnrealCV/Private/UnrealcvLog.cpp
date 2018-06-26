// Weichao Qiu @ 2018
#include "UnrealcvLog.h"
#include "Runtime/Engine/Classes/Engine/Engine.h"

void ScreenLog(const FString& Message)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Message);
}