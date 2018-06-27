// Weichao Qiu @ 2018
#pragma once

#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "JsonObjectBP.h"
#include "SerializeBPLib.generated.h"

/** A static BP library to serialize data to string, mainly for json */
UCLASS()
class UNREALCV_API USerializeBPLib : public UBlueprintFunctionLibrary
{
	// This BP funtion library exists as a wrapper for Autocast
	GENERATED_BODY()

public:
	/** Convert to json */
	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP FloatToJson(float Value);

	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP IntToJson(int Value);

	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP StringToJson(const FString& Value);

	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP VectorToJson(const FVector& Vec);

	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP RotatorToJson(const FRotator& Rotator);

	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP ColorToJson(const FColor& Color);

	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP TransformToJson(const FTransform& Transform);
	// Convert string array to json
	// UFUNCTION(BlueprintPure, Category = "unrealcv")
	// static FString StrArrayToJson(const TArray<FString>& Array);
	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP ArrayToJson(const TArray<FJsonObjectBP>& Array);

	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP TMapToJson(const TArray<FString>& Keys, const TArray<FJsonObjectBP>& Values); // TMap is not supported in BP

	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FJsonObjectBP StringMapToJson(const TArray<FString>& Keys, const TArray<FString>& Values);

	// Automatically cast json object to string
	UFUNCTION(BlueprintPure, meta=(BlueprintAutocast), Category = "unrealcv")
	static FString JsonToStr(const FJsonObjectBP& JsonObjectBP);


};
