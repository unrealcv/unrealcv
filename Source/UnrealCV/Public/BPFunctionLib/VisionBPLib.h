// Weichao Qiu @ 2018
#pragma once

#include "SerializeBPLib.h"
#include "Runtime/Engine/Classes/Kismet/BlueprintFunctionLibrary.h"
#include "VisionBPLib.generated.h"

UENUM(BlueprintType)
enum class EFileFormat : uint8
{
	Png	UMETA(DisplayName="Png"),
	Npy	UMETA(DisplayName="Npy"),
};

/** A static bp library for computer vision */
UCLASS()
class UNREALCV_API UVisionBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// UFUNCTION(BlueprintCallable, Category = "unrealcv")
	// static FString SerializeBoneInfo(USkeletalMeshComponent* Component);
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static int FrameNumber()
	{
		return GFrameNumber;
	}

	// Create an empty
	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static bool CreateFile(const FString& Filename);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static bool SaveData(const FString& Data, const FString& Filename);

	// Append data to an existing file
	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static bool AppendData(const FString& Data, const FString& Filename);

	// FilenameFmt is the format string which contains %d
	// UFUNCTION(BlueprintCallable, Category = "unrealcv")
	// static FString FormatFrameFilename(FString FilenameFmt);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static bool SendMessageBP(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static void SavePng(const TArray<FColor>& ImageData, int Width, int Height, FString Filename, bool bKeepAlpha = false);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static void SaveNpy(const TArray<float>& FloatData, int Width, int Height, FString Filename);

	// Extract SkeletalMesh bone information
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static void GetBoneTransform(
		const USkeletalMeshComponent* SkeletalMeshComponent,
		const TArray<FString>& IncludedBones,
		TArray<FString>& BoneNames,
		TArray<FTransform>& BoneTransforms,
		bool bWorldSpace = false);

	// Get bone transformation and return as JsonObject array, this can make BP programming much easier
	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static void GetBoneTransformJson(
		const USkeletalMeshComponent* SkeletalMeshComponent,
		const TArray<FString>& IncludedBones,
		TArray<FString>& BoneNames,
		TArray<FJsonObjectBP>& BoneTransformsJson,
		bool bWorldSpace = false
	);

	// Extract vertex data of a mesh
	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static void GetVertexArray(const AActor* Actor, TArray<FVector>& VertexArray);

	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static void UpdateInput(APawn* Pawn, bool Enable);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static void GetActorList(TArray<AActor*>& ActorList);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static void GetAnnotationColor(AActor* Actor, FColor& AnnotationColor);

	/**
	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static void AnnotateWorld();
	*/

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static TArray<FVector> SkinnedMeshComponentGetVertexArray(USkinnedMeshComponent* Component);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static TArray<FVector> StaticMeshComponentGetVertexArray(UStaticMeshComponent* StaticMeshComponent);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static TArray<FVector> GetVertexArrayFromMeshComponent(UMeshComponent* MeshComponent);

	UFUNCTION(BlueprintPure, Category = "unrealcv")
	static UFusionCamSensor* GetPlayerSensor();

	/** If there is manually crated objects, 
	 * need to manually call this function to annotate the world */
	UFUNCTION(BlueprintCallable, Category = "unrealcv")
	static void AnnotateWorld();
};
