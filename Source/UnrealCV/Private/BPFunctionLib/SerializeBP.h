// Weichao Qiu @ 2018
#pragma once
#include "SerializeBP.generated.h"

// Make JsonValue accessible in blueprint
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

	FJsonObjectBP(float Value)
	{
		this->JsonValue = MakeShareable(new FJsonValueNumber(Value));
	}

	FJsonObjectBP(int Value)
	{
		this->JsonValue = MakeShareable(new FJsonValueNumber(Value));
	}

	FJsonObjectBP(const FString& String)
	{
		this->JsonValue = MakeShareable(new FJsonValueString(String));
	}

	FJsonObjectBP(const FVector& Vector)
	{
		JsonObject = MakeShareable(new FJsonObject());
		JsonObject->SetNumberField("X", Vector.X);
		JsonObject->SetNumberField("Y", Vector.Y);
		JsonObject->SetNumberField("Z", Vector.Z);
	}

	FJsonObjectBP(const FRotator& Rotator)
	{
		JsonObject = MakeShareable(new FJsonObject());
		JsonObject->SetNumberField("Pitch", Rotator.Pitch);
		JsonObject->SetNumberField("Yaw", Rotator.Yaw);
		JsonObject->SetNumberField("Roll", Rotator.Roll);
	}

	FJsonObjectBP(const FColor& Color)
	{
		JsonObject = MakeShareable(new FJsonObject());
		JsonObject->SetNumberField("R", Color.R);
		JsonObject->SetNumberField("G", Color.G);
		JsonObject->SetNumberField("B", Color.B);
	}

	FJsonObjectBP(const FTransform& Transform)
	{
		JsonObject = MakeShareable(new FJsonObject());
		JsonObject->SetNumberField("Pitch", Transform.GetRotation().Rotator().Pitch);
		JsonObject->SetNumberField("Yaw", Transform.GetRotation().Rotator().Yaw);
		JsonObject->SetNumberField("Roll", Transform.GetRotation().Rotator().Roll);
		JsonObject->SetNumberField("X", Transform.GetTranslation().X);
		JsonObject->SetNumberField("Y", Transform.GetTranslation().Y);
		JsonObject->SetNumberField("Z", Transform.GetTranslation().Z);
		JsonObject->SetNumberField("ScaleX", Transform.GetScale3D().X);
		JsonObject->SetNumberField("ScaleY", Transform.GetScale3D().Y);
		JsonObject->SetNumberField("ScaleZ", Transform.GetScale3D().Z);

	}

	FJsonObjectBP(const TArray<FJsonObjectBP>& Array)
	{
		for (FJsonObjectBP Value : Array)
		{
			JsonArray.Add(Value.ToJsonValue());
		}
	}

	FJsonObjectBP(const TArray<FString>& Keys, const TArray<FString>& Values)
	{
		JsonObject = MakeShareable(new FJsonObject());
		if (Keys.Num() != Values.Num())
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Length of keys and values are mismatch"));
			return;
		}

		for (int i = 0; i < Keys.Num(); i++)
		{
			JsonObject->SetStringField(Keys[i], Values[i]);
		}
	}

	FJsonObjectBP(const TArray<FString>& Keys, const TArray<FJsonObjectBP>& Values)
	{
		JsonObject = MakeShareable(new FJsonObject());
		if (Keys.Num() != Values.Num())
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Length of keys and values are mismatch"));
			return;
		}

		for (int i = 0; i < Keys.Num(); i++)
		{
			JsonObject->SetField(Keys[i], Values[i].ToJsonValue());
		}
	}

	TSharedPtr<FJsonValue> ToJsonValue() const
	{
		if (this->JsonObject.IsValid())
		{
			return MakeShareable(new FJsonValueObject(JsonObject));
		}
		else if (this->JsonValue.IsValid())
		{
			return this->JsonValue;
		}

		// Assume this object is empty or an array
		return MakeShareable(new FJsonValueArray(JsonArray));
	}

};

/** A static BP library to serialize data to string, mainly for json */
UCLASS()
class UNREALCV_API USerializeBP : public UBlueprintFunctionLibrary
{
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
