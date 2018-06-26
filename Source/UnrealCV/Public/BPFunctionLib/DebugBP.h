// Weichao Qiu @ 2017
#pragma once
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "DebugBP.generated.h"

UCLASS()
class UDebugBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	static void PrintAllActors();
};
