// Weichao Qiu @ 2018
#pragma once

#include "Runtime/Json/Public/Dom/JsonObject.h"
#include "JsonObjectBP.generated.h"

/** Make JsonValue accessible in blueprint */
USTRUCT(BlueprintType)
struct UNREALCV_API FJsonObjectBP
{
	GENERATED_BODY()

	TSharedPtr<FJsonObject> JsonObject;
	// Json value can not be serialized directly
	TSharedPtr<FJsonValue> JsonValue;
	TArray<TSharedPtr<FJsonValue> > JsonArray;
	// TSharedPtr<FJsonValueArray> JsonArray;

	FJsonObjectBP() {}

	FJsonObjectBP(float Value);

	FJsonObjectBP(int Value);

	FJsonObjectBP(const FString& String);

	FJsonObjectBP(const FVector& Vector);

	FJsonObjectBP(const FRotator& Rotator);

	FJsonObjectBP(const FColor& Color);

	FJsonObjectBP(const FTransform& Transform);

	FJsonObjectBP(const TArray<FString>& StrArray);

	FJsonObjectBP(const TArray<FJsonObjectBP>& Array);

	FJsonObjectBP(const TArray<FString>& Keys, const TArray<FJsonObjectBP>& Values);

	FJsonObjectBP(const TArray<FString>& Keys, const TArray<FString>& Values);

	FJsonObjectBP(const TMap<FString, float>& Dict);

	FJsonObjectBP(const TMap<FString, FString>& Dict);

	FJsonObjectBP(const TMap<FString, FJsonObjectBP>& Dict);

	TSharedPtr<FJsonValue> ToJsonValue() const;

	FString ToString() const;
};
