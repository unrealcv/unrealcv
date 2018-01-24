// Weichao Qiu @ 2017
#pragma once
#include "JsonObject.h"

/** Serialize data to json format */
class FJsonFormatter : public FArchive
{
public:
	FJsonFormatter()
	{
		JsonObject = MakeShareable(new FJsonObject);
	}

	TSharedPtr<FJsonObject> JsonObject;

	FString ToString()
	{
		// example from http://www.wraiyth.com/?p=198
		FString OutputString;
		TSharedRef<TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
		return OutputString;
	}
};

inline FJsonFormatter& operator<<(FJsonFormatter& Ar, const FVector& Vec)
{
	Ar.JsonObject->SetNumberField("X", Vec.X);
	Ar.JsonObject->SetNumberField("Y", Vec.Y);
	Ar.JsonObject->SetNumberField("Z", Vec.Z);
	return Ar;
}

inline FJsonFormatter& operator<<(FJsonFormatter& Ar, const FRotator& Rotator)
{
	Ar.JsonObject->SetNumberField("Pitch", Rotator.Pitch);
	Ar.JsonObject->SetNumberField("Yaw", Rotator.Yaw);
	Ar.JsonObject->SetNumberField("Roll", Rotator.Roll);
	return Ar;
}

inline FJsonFormatter& operator<<(FJsonFormatter& Ar, const FColor& Color)
{
	Ar.JsonObject->SetNumberField("R", Color.R);
	Ar.JsonObject->SetNumberField("G", Color.G);
	Ar.JsonObject->SetNumberField("B", Color.B);
	return Ar;
}


inline FJsonFormatter& operator<<(FJsonFormatter& Ar, const TMap<FString, FString>& Dict)
{
	for (const auto& Elem: Dict)
	{
		Ar.JsonObject->SetStringField(Elem.Key, Elem.Value);
	}
	return Ar;
}
