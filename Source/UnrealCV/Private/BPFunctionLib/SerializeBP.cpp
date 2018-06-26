// Weichao Qiu @ 2018
#include "SerializeBP.h"
#include "UnrealCVPrivate.h"
#include "JsonObject.h"
#include "Runtime/Json/Public/Serialization/JsonSerializer.h"

FJsonObjectBP::FJsonObjectBP(const TArray<FString>& Keys, const TArray<FString>& Values)
{
	JsonObject = MakeShareable(new FJsonObject());
	if (Keys.Num() != Values.Num())
	{
		// UE_LOG(LogUnrealCV, Warning, TEXT("Length of keys and values are mismatch"));
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
		// UE_LOG(LogUnrealCV, Warning, TEXT("Length of keys and values are mismatch"));
		return;
	}

	for (int i = 0; i < Keys.Num(); i++)
	{
		JsonObject->SetField(Keys[i], Values[i].ToJsonValue());
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


FJsonObjectBP USerializeBP::FloatToJson(float Value)
{
	return FJsonObjectBP(Value);
}

FJsonObjectBP USerializeBP::IntToJson(int Value)
{
	return FJsonObjectBP(Value);
}

FJsonObjectBP USerializeBP::StringToJson(const FString& Value)
{
	return FJsonObjectBP(Value);
}

FJsonObjectBP USerializeBP::VectorToJson(const FVector& Vec)
{
	FJsonObjectBP JsonObjectBP(Vec);
	return JsonObjectBP;
}

FJsonObjectBP USerializeBP::RotatorToJson(const FRotator& Rotator)
{
	FJsonObjectBP JsonObjectBP(Rotator);
	return JsonObjectBP;
}

FJsonObjectBP USerializeBP::ColorToJson(const FColor& Color)
{
	FJsonObjectBP JsonObjectBP(Color);
	return JsonObjectBP;
}

FJsonObjectBP USerializeBP::TransformToJson(const FTransform& Transform)
{
	FJsonObjectBP JsonObjectBP(Transform);
	return JsonObjectBP;
}

FJsonObjectBP USerializeBP::ArrayToJson(const TArray<FJsonObjectBP>& Array)
{
	FJsonObjectBP JsonObjectBP(Array);
	return JsonObjectBP;
}

/** Convert string array to json */
// FString USerializeBP::StrArrayToJson(const TArray<FString>& StrArray)
// {
// 	TSharedRef<FJsonObject> JsonObject(new FJsonObject());
// 	TArray<TSharedPtr<FJsonValue> > JsonStrArray;
//
// 	for (FString Value : StrArray)
// 	{
// 		JsonStrArray.Add(MakeShareable(new FJsonValueString(Value)));
// 	}
//
// 	FString OutputString;
// 	TSharedRef<TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
// 	FJsonSerializer::Serialize(JsonStrArray, Writer);
// 	return OutputString;
// }

FJsonObjectBP USerializeBP::TMapToJson(const TArray<FString>& Keys, const TArray<FJsonObjectBP>& Values)
{
	FJsonObjectBP JsonObjectBP(Keys, Values);
	return JsonObjectBP;
}

// BP function does not support function overload, so we need another function name
FJsonObjectBP USerializeBP::StringMapToJson(const TArray<FString>& Keys, const TArray<FString>& Values)
{
	FJsonObjectBP JsonObjectBP(Keys, Values);
	return JsonObjectBP;
}

FString USerializeBP::JsonToStr(const FJsonObjectBP& JsonObjectBP)
{
	return JsonObjectBP.ToString();
}
