// Weichao Qiu @ 2018
#include "UnrealCVPrivate.h"
#include "SerializeBP.h"
#include "JsonFormatter.h"
#include "JsonObject.h"

// FString USerializeBP::VectorToJson(const FVector& Vec)
// {
// 	FJsonFormatter Ar;
// 	Ar << Vec;
// 	return Ar.ToString();
// }

// FString USerializeBP::RotatorToJson(const FRotator& Rotator)
// {
// 	FString JsonStr;
// 	return JsonStr;
// }
//
// FString USerializeBP::ColorToJson(const FColor& Color)
// {
// 	FJsonFormatter Ar;
// 	Ar << Color;
// 	return Ar.ToString();
// }

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
	FString OutputString;
	TSharedRef<TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
	if (JsonObjectBP.JsonObject.IsValid())
	{
		FJsonSerializer::Serialize(JsonObjectBP.JsonObject.ToSharedRef(), Writer);
	}
	else
	{
		FJsonSerializer::Serialize(JsonObjectBP.JsonArray, Writer);
	}
	return OutputString;
}
