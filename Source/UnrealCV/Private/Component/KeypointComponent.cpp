// Weichao Qiu @ 2018

#include "UnrealCVPrivate.h"
#include "KeypointComponent.h"
#include "VisionBP.h"

UKeypointComponent::UKeypointComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	this->PrimaryComponentTick.bCanEverTick = true;

}

void UKeypointComponent::OnRegister()
{
	Super::OnRegister();
	this->Keypoints = LoadKeypointFromJson(this->JsonFilename);

	if (bMatchNearestVertex)
	{
		MatchNearestVertex();
	}
}

void UKeypointComponent::MatchNearestVertex()
{
	AActor* OwnerActor = this->GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}
	TArray<UActorComponent*> MeshComponents = OwnerActor->GetComponentsByClass(UMeshComponent::StaticClass());
	MatchedVertexs.Empty();
	
	// TODO: Speedup this slow implementation
	for (FKeypoint& Keypoint : Keypoints)
	{
		FMatchedVertexInfo VertexInfo;
		for (UActorComponent* Component : MeshComponents)
		{
			UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component);
			TArray<FVector> VertexArray = UVisionBP::GetVertexArrayFromMeshComponent(MeshComponent);

			double MinDistance = 10e10;
			int VertexIndex;
			for (int i = 0; i < VertexArray.Num(); i++)
			{
				FVector Vertex = VertexArray[i];
				double Distance = FVector::Distance(Vertex, Keypoint.Location);
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

void UKeypointComponent::PostInitProperties()
{
	Super::PostInitProperties();
}

void UKeypointComponent::PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}


TArray<FKeypoint> UKeypointComponent::LoadKeypointFromJson(FString JsonFilename)
{
	// Load annotation from json file
	// TODO: Cached the loaded keypoints, because there might be many similar instances
	TArray<FKeypoint> Points;
	FString JsonString;
	FFileHelper::LoadFileToString(JsonString, *JsonFilename);
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
		if (!bMatchNearestVertex)
		{
			for (FKeypoint& Keypoint : Keypoints)
			{
				FVector WorldPointLocation = this->GetComponentRotation().RotateVector(Keypoint.Location) + this->GetComponentLocation(); // in world space
				DrawDebugPoint(GetWorld(), WorldPointLocation, VisualizePointSize, FColor::Red, false, DeltaTime);

				if (bDrawKeypointName)
				{
					DrawDebugString(GetWorld(), WorldPointLocation, Keypoint.Name, nullptr, FColor::Green, DeltaTime);
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
				TArray<FVector> VertexArray = UVisionBP::GetVertexArrayFromMeshComponent(MeshComponent);
				if (VertexInfo.VertexIndex < 0 || VertexInfo.VertexIndex >= VertexArray.Num())
				{
					UE_LOG(LogUnrealCV, Warning, TEXT("Unexpected error in the MatchedVertexInfo"));
					continue;
				}

				FVector VertexLocation = VertexArray[VertexInfo.VertexIndex];
				FVector WorldPointLocation = MeshComponent->GetComponentRotation().RotateVector(VertexLocation) + MeshComponent->GetComponentLocation(); // in world space
				DrawDebugPoint(GetWorld(), WorldPointLocation, VisualizePointSize, FColor::Green, false, DeltaTime);

				if (bDrawKeypointName)
				{
					DrawDebugString(GetWorld(), WorldPointLocation, VertexInfo.KeypointName, nullptr, FColor::Green, DeltaTime);
				}
			}
		}
	}
}