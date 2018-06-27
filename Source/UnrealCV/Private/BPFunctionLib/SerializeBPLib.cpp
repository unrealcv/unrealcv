// Weichao Qiu @ 2018
#include "SerializeBPLib.h"
#include "Runtime/Json/Public/Serialization/JsonSerializer.h"

FJsonObjectBP USerializeBPLib::FloatToJson(float Value)
{
	return FJsonObjectBP(Value);
}

FJsonObjectBP USerializeBPLib::IntToJson(int Value)
{
	return FJsonObjectBP(Value);
}

FJsonObjectBP USerializeBPLib::StringToJson(const FString& Value)
{
	return FJsonObjectBP(Value);
}

FJsonObjectBP USerializeBPLib::VectorToJson(const FVector& Vec)
{
	return FJsonObjectBP(Vec);
}

FJsonObjectBP USerializeBPLib::RotatorToJson(const FRotator& Rotator)
{
	return FJsonObjectBP(Rotator);
}

FJsonObjectBP USerializeBPLib::ColorToJson(const FColor& Color)
{
	return FJsonObjectBP(Color);
}

FJsonObjectBP USerializeBPLib::TransformToJson(const FTransform& Transform)
{
	return FJsonObjectBP(Transform);
}

FJsonObjectBP USerializeBPLib::ArrayToJson(const TArray<FJsonObjectBP>& Array)
{
	return FJsonObjectBP(Array);
}

/** Convert string array to json */
// FString USerializeBPLib::StrArrayToJson(const TArray<FString>& StrArray)
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

FJsonObjectBP USerializeBPLib::TMapToJson(const TArray<FString>& Keys, const TArray<FJsonObjectBP>& Values)
{
	FJsonObjectBP JsonObjectBP(Keys, Values);
	return JsonObjectBP;
}

// BP function does not support function overload, so we need another function name
FJsonObjectBP USerializeBPLib::StringMapToJson(const TArray<FString>& Keys, const TArray<FString>& Values)
{
	FJsonObjectBP JsonObjectBP(Keys, Values);
	return JsonObjectBP;
}

FString USerializeBPLib::JsonToStr(const FJsonObjectBP& JsonObjectBP)
{
	return JsonObjectBP.ToString();
}
