// Weichao Qiu @ 2018
#pragma once

#include "Runtime/Engine/Classes/Components/MeshComponent.h"
#include "SerializeBPLib.h"
#include "KeypointComponent.generated.h"

struct FMatchedVertexInfo
{
	UMeshComponent* MeshComponent;
	AActor* Actor;
	int VertexIndex;
	double Distance;
	FString KeypointName;

	FMatchedVertexInfo() : Distance(10e10)
	{
	}

	FMatchedVertexInfo(UMeshComponent* InComponent, AActor* InActor, 
		int InVertexIndex, double InDistance, FString InKeypointName) :
		MeshComponent(InComponent), Actor(InActor), 
		VertexIndex(InVertexIndex), Distance(InDistance),
		KeypointName(InKeypointName)
	{
	}
};

struct FKeypoint
{
	FVector Location;
	FString Name;
};

UCLASS(meta = (BlueprintSpawnableComponent))
class UKeypointComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UKeypointComponent(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unrealcv")
	FString JsonFilename;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unrealcv")
	bool bVisualize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unrealcv")
	float VisualizePointSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unrealcv")
	bool bMatchNearestVertex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "unrealcv")
	bool bDrawKeypointName;

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetKeypoints(
		TArray<FString>& KeypointNames, 
		TArray<FVector>& Locations, 
		bool bWorldSpace = false);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	void GetKeypointsJson(
		TArray<FString>& KeypointNames, 
		TArray<FJsonObjectBP>& LocationsBP, 
		bool bWorldSpace = false);

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* T) override;

	// Only available in the editor mode
	// virtual void PostInitProperties() override;
	// virtual void PostEditChangeProperty(FPropertyChangedEvent & PropertyChangedEvent) override;

	virtual void OnRegister() override;

	virtual void BeginPlay() override;

	void MatchNearestVertex();

private:
	TArray<FMatchedVertexInfo> MatchedVertexs;

	TArray<FKeypoint> Keypoints;

	TArray<FKeypoint> LoadKeypointFromJson();
};