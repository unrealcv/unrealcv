// Weichao Qiu @ 2018
#include "JsonObjectBP.h"
#include "Runtime/Json/Public/Serialization/JsonSerializer.h"
#include "UnrealcvLog.h"

FJsonObjectBP::FJsonObjectBP(float Value)
{
	this->JsonValue = MakeShareable(new FJsonValueNumber(Value));
}

FJsonObjectBP::FJsonObjectBP(int Value)
{
	this->JsonValue = MakeShareable(new FJsonValueNumber(Value));
}

FJsonObjectBP::FJsonObjectBP(const FString& String)
{
	this->JsonValue = MakeShareable(new FJsonValueString(String));
}

FJsonObjectBP::FJsonObjectBP(const FVector& Vector)
{
	JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetNumberField("X", Vector.X);
	JsonObject->SetNumberField("Y", Vector.Y);
	JsonObject->SetNumberField("Z", Vector.Z);
}

FJsonObjectBP::FJsonObjectBP(const FRotator& Rotator)
{
	JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetNumberField("Pitch", Rotator.Pitch);
	JsonObject->SetNumberField("Yaw", Rotator.Yaw);
	JsonObject->SetNumberField("Roll", Rotator.Roll);
}

FJsonObjectBP::FJsonObjectBP(const FColor& Color)
{
	JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetNumberField("R", Color.R);
	JsonObject->SetNumberField("G", Color.G);
	JsonObject->SetNumberField("B", Color.B);
}

FJsonObjectBP::FJsonObjectBP(const FTransform& Transform)
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

FJsonObjectBP::FJsonObjectBP(const TArray<FString>& StrArray)
{
	for (FString Value : StrArray)
	{
		JsonArray.Add(MakeShareable(new FJsonValueString(Value)));
	}
}

FJsonObjectBP::FJsonObjectBP(const TArray<FJsonObjectBP>& Array)
{
	for (FJsonObjectBP Value : Array)
	{
		JsonArray.Add(Value.ToJsonValue());
	}
}

FJsonObjectBP::FJsonObjectBP(const TArray<FString>& Keys, const TArray<FString>& Values)
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

FJsonObjectBP::FJsonObjectBP(const TArray<FString>& Keys, const TArray<FJsonObjectBP>& Values)
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

FJsonObjectBP::FJsonObjectBP(const TMap<FString, float>& Dict) 
{
	JsonObject = MakeShareable(new FJsonObject());
	for (const auto& Elem: Dict)
	{
		JsonObject->SetNumberField(Elem.Key, Elem.Value);
	}
}

FJsonObjectBP::FJsonObjectBP(const TMap<FString, FString>& Dict) 
{
	JsonObject = MakeShareable(new FJsonObject());
	for (const auto& Elem: Dict)
	{
		JsonObject->SetStringField(Elem.Key, Elem.Value);
	}
}

FJsonObjectBP::FJsonObjectBP(const TMap<FString, FJsonObjectBP>& Dict)
{
	JsonObject = MakeShareable(new FJsonObject());
	for (const auto& Elem: Dict)
	{
		JsonObject->SetField(Elem.Key, Elem.Value.ToJsonValue());
	}
}

TSharedPtr<FJsonValue> FJsonObjectBP::ToJsonValue() const
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

FString FJsonObjectBP::ToString() const
{
	FString OutputString;
	TSharedRef<TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (this->JsonObject.IsValid())
	{
		FJsonSerializer::Serialize(this->JsonObject.ToSharedRef(), Writer);
	}
	else
	{
		FJsonSerializer::Serialize(this->JsonArray, Writer);
	}
	return OutputString;
}
