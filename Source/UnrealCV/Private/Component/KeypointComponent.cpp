// Weichao Qiu @ 2018

#include "KeypointComponent.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "Runtime/Core/Public/Misc/Paths.h"
#include "Runtime/Json/Public/Serialization/JsonReader.h"
#include "Runtime/Json/Public/Serialization/JsonSerializer.h"
#include "VisionBPLib.h"
#include "UnrealcvLog.h"

UKeypointComponent::UKeypointComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->PrimaryComponentTick.bCanEverTick = true;

}

// When OnRegister the mesh might not be properly loaded
void UKeypointComponent::OnRegister()
{
	Super::OnRegister();
}

void UKeypointComponent::BeginPlay()
{
	Super::BeginPlay();
	this->Keypoints = LoadKeypointFromJson();

	if (bMatchNearestVertex)
	{
		MatchNearestVertex();
	}
}

// Support skeletal mesh and actor with many MeshComponents
void UKeypointComponent::MatchNearestVertex()
{
	AActor* OwnerActor = this->GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}
	TArray<UActorComponent*> MeshComponents = OwnerActor->GetComponentsByClass(UMeshComponent::StaticClass());
	MatchedVertexs.Empty();
	
	// Note: Match the keypoint in the actor space, not in the component local space
	// Note: Match in the component space is also doable, but has less value
	// TODO: Speedup this slow implementation
	for (FKeypoint& Keypoint : Keypoints)
	{
		FMatchedVertexInfo VertexInfo;
		for (UActorComponent* Component : MeshComponents)
		{
			UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component);
			TArray<FVector> VertexArray = UVisionBPLib::GetVertexArrayFromMeshComponent(MeshComponent);

			double MinDistance = 10e10;
			int VertexIndex = -1;
			for (int i = 0; i < VertexArray.Num(); i++)
			{
				FVector Vertex = VertexArray[i];
				FVector VertexWorld = MeshComponent->GetComponentRotation().RotateVector(Vertex) + MeshComponent->GetComponentLocation() - OwnerActor->GetActorLocation();
				// Change from component space to world space
				double Distance = FVector::Distance(VertexWorld, Keypoint.Location);
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					VertexIndex = i;
				}
			}
			if (MinDistance < VertexInfo.Distance)
			{
				VertexInfo = FMatchedVertexInfo(MeshComponent, OwnerActor, VertexIndex, 
					MinDistance, Keypoint.Name);
			}
		}
		MatchedVertexs.Add(VertexInfo);
	}
}

TArray<FKeypoint> UKeypointComponent::LoadKeypointFromJson()
{
	TArray<FKeypoint> Points;
	if (JsonFilename.IsEmpty())
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Keypoint annotation filename is empty for actor %s"), *this->GetOwner()->GetName());
		return Points;
	}
	// Load annotation from json file
	// TODO: Cached the loaded keypoints, because there might be many similar instances
	FString JsonString;

	FString GameDir = FPaths::ProjectDir();
	FString GameContentDir = FPaths::ProjectContentDir();
	if (!FPaths::FileExists(JsonFilename))
	{
		JsonFilename = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), JsonFilename));
	}

	FFileHelper::LoadFileToString(JsonString, *JsonFilename);
	UE_LOG(LogUnrealCV, Log, TEXT("Try to load keypoint annotation from file %s"), *JsonFilename);
	if (JsonString.IsEmpty() || JsonFilename.IsEmpty())
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not read data from file %s"), *JsonFilename);
		return Points;
	}

	// TSharedPtr<FJsonObject> JsonObject;
	TArray<TSharedPtr<FJsonValue>> JsonArray;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	bool bParseStatus = FJsonSerializer::Deserialize(Reader, JsonArray);
	if (!bParseStatus)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not parse json when reading keypoint from Json file %s"), *JsonFilename);
		return Points;
	}

	for (TSharedPtr<FJsonValue>& JsonValue : JsonArray)
	{
		const TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
		double X = JsonObject->GetNumberField("x");
		double Y = JsonObject->GetNumberField("y");
		double Z = JsonObject->GetNumberField("z");
		FKeypoint Point;
		Point.Location = FVector(X, Y, Z);
		Point.Name = JsonObject->GetStringField("name");
		Points.Add(Point);
	}
	return Points;
}

void UKeypointComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* T)
{
	// Draw debug points, the annotation is in the local space
	if (this->bVisualize)
	{
		TArray<FString> KeypointNames;
		TArray<FVector> Locations;
		if (KeypointNames.Num() != Locations.Num())
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("The keypoint name and location array have different length"));
			return;
		}
		this->GetKeypoints(KeypointNames, Locations, true);
		for (int i = 0; i < KeypointNames.Num(); i++)
		{
			DrawDebugPoint(GetWorld(), Locations[i], VisualizePointSize, FColor::Red, false, DeltaTime);
			if (bDrawKeypointName)
			{
				DrawDebugString(GetWorld(), Locations[i], KeypointNames[i], nullptr, FColor::Green, DeltaTime);
			}
		}
		// if (!bMatchNearestVertex)
		// {
		// 	for (FKeypoint& Keypoint : Keypoints)
		// 	{
		// 		FVector WorldPointLocation = this->GetComponentRotation().RotateVector(Keypoint.Location) + this->GetComponentLocation(); // in world space
		// 		DrawDebugPoint(GetWorld(), WorldPointLocation, VisualizePointSize, FColor::Red, false, DeltaTime);

		// 		if (bDrawKeypointName)
		// 		{
		// 			DrawDebugString(GetWorld(), WorldPointLocation, Keypoint.Name, nullptr, FColor::Green, DeltaTime);
		// 		}
		// 	}
		// }
		
		// if (bMatchNearestVertex)
		// {
		// 	for (FMatchedVertexInfo& VertexInfo : this->MatchedVertexs)
		// 	{
		// 		if (!IsValid(VertexInfo.Actor) || !IsValid(VertexInfo.MeshComponent))
		// 		{
		// 			continue;
		// 		}

		// 		UMeshComponent* MeshComponent = VertexInfo.MeshComponent;
		// 		TArray<FVector> VertexArray = UVisionBPLib::GetVertexArrayFromMeshComponent(MeshComponent);
		// 		if (VertexInfo.VertexIndex < 0 || VertexInfo.VertexIndex >= VertexArray.Num())
		// 		{
		// 			UE_LOG(LogUnrealCV, Warning, TEXT("Unexpected error in the MatchedVertexInfo"));
		// 			continue;
		// 		}

		// 		FVector VertexLocation = VertexArray[VertexInfo.VertexIndex];
		// 		FVector WorldPointLocation = MeshComponent->GetComponentRotation().RotateVector(VertexLocation) + MeshComponent->GetComponentLocation(); // in world space
		// 		DrawDebugPoint(GetWorld(), WorldPointLocation, VisualizePointSize, FColor::Green, false, DeltaTime);

		// 		if (bDrawKeypointName)
		// 		{
		// 			DrawDebugString(GetWorld(), WorldPointLocation, VertexInfo.KeypointName, nullptr, FColor::Green, DeltaTime);
		// 		}
		// 	}
		// }
	}
}

void UKeypointComponent::GetKeypoints(
	TArray<FString>& KeypointNames, 
	TArray<FVector>& Locations, 
	bool bWorldSpace)
{
	if (!bMatchNearestVertex)
	{
		for (FKeypoint& Keypoint : Keypoints)
		{
			KeypointNames.Add(Keypoint.Name);
			if (bWorldSpace == false)
			{
				FVector WorldPointLocation = this->GetComponentRotation().RotateVector(Keypoint.Location) + this->GetComponentLocation(); // in world space
				Locations.Add(WorldPointLocation);
			}
			else
			{
				Locations.Add(Keypoint.Location);
			}
		}
	}
	
	if (bMatchNearestVertex)
	{
		for (FMatchedVertexInfo& VertexInfo : this->MatchedVertexs)
		{
			if (!IsValid(VertexInfo.Actor) || !IsValid(VertexInfo.MeshComponent))
			{
				continue;
			}

			UMeshComponent* MeshComponent = VertexInfo.MeshComponent;
			TArray<FVector> VertexArray = UVisionBPLib::GetVertexArrayFromMeshComponent(MeshComponent);
			if (VertexInfo.VertexIndex < 0 || VertexInfo.VertexIndex >= VertexArray.Num())
			{
				UE_LOG(LogUnrealCV, Warning, TEXT("Unexpected error in the MatchedVertexInfo"));
				continue;
			}

			KeypointNames.Add(VertexInfo.KeypointName);
			if (bWorldSpace == false)
			{
				FVector VertexLocation = VertexArray[VertexInfo.VertexIndex];
				Locations.Add(VertexLocation);
			}
			else
			{
				FVector VertexLocation = VertexArray[VertexInfo.VertexIndex];
				FVector WorldPointLocation = MeshComponent->GetComponentRotation().RotateVector(VertexLocation) + MeshComponent->GetComponentLocation(); // in world space
				Locations.Add(WorldPointLocation);
			}
		}
	}
}

void UKeypointComponent::GetKeypointsJson(
	TArray<FString>& KeypointNames, 
	TArray<FJsonObjectBP>& LocationsBP, 
	bool bWorldSpace)
{
	TArray<FVector> Locations;
	GetKeypoints(KeypointNames, Locations, bWorldSpace);
	for (FVector& Location : Locations)
	{
		FJsonObjectBP JsonObjectBP(Location);
		LocationsBP.Add(JsonObjectBP);
	}
}