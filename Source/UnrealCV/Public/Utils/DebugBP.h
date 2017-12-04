// Weichao Qiu @ 2017

#include "DebugBP.generated.h"

UCLASS()
class UDebugBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	static void PrintAllActors();
};
