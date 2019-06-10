// Weichao Qiu @ 2017
#include "VisionBPLib.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshLODRenderData.h"
#include "Runtime/Engine/Public/Rendering/SkeletalMeshRenderData.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Public/SkeletalRenderPublic.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"

#include "ImageUtil.h"
#include "UnrealcvServer.h"
#include "Serialization.h"
#include "VertexSensor.h"
#include "BoneSensor.h"
#include "FusionCamSensor.h"
#include "UnrealcvLog.h"
#include "WorldController.h"

// TODO: Move features outside blueprint function library.

// FString UVisionBPLib::SerializeBoneInfo(USkeletalMeshComponent* Component)
// {
// 	FString Data;
// 	TArray<FBoneIndexType> RequiredBones = Component->RequiredBones;
// 	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
// 	auto ComponentSpaceTransforms = Component->GetComponentSpaceTransforms();
// 	FTransform ComponentToWorld = Component->ComponentToWorld;
//
// 	for (int32 Index = 0; Index < RequiredBones.Num(); ++Index)
// 	{
// 		int32 BoneIndex = RequiredBones[Index];
// 		// FReferenceSkeleton
// 		int32 ParentIndex = SkeletalMesh->RefSkeleton.GetParentIndex(BoneIndex);
//
// 		FVector Start, End;
// 		FTransform BoneTM = ComponentSpaceTransforms[BoneIndex] * ComponentToWorld;
//
// 		End = BoneTM.GetLocation();
// 		if (ParentIndex >= 0)
// 		{
// 			Start = (ComponentSpaceTransforms[ParentIndex] * ComponentToWorld).GetLocation();
// 		}
// 		else // No parent??
// 		{
// 			Start = ComponentToWorld.GetLocation();
// 		}
// 		FString BoneName = SkeletalMesh->RefSkeleton.GetBoneName(BoneIndex).ToString();
// 		UE_LOG(LogTemp, Log, TEXT("Bone: %d, %s, Start: %s, End: %s"), BoneIndex, *BoneName, *Start.ToString(), *End.ToString());
// 		Data += FString::Printf(TEXT("Bone: %d, %s, Start: %s, End: %s\r\n"), BoneIndex, *BoneName, *Start.ToString(), *End.ToString());
// 	}
//
// 	return Data;
// }

bool UVisionBPLib::CreateFile(const FString& Filename)
{
	if (FFileHelper::SaveStringToFile("", *Filename))
	{
		return true;
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can create file %s"), *Filename);
		return false;
	}
}

bool UVisionBPLib::SaveData(const FString& Data, const FString& Filename)
{
	if (FFileHelper::SaveStringToFile(Data, *Filename))
	{
		return true;
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not save to file %s"), *Filename);
		return false;
	}
}

bool UVisionBPLib::AppendData(const FString& Data, const FString& Filename)
{
	// https://answers.unrealengine.com/questions/250210/append-to-text-file-instead-of-overwrite-whats-the.html
	if (FFileHelper::SaveStringToFile(Data, *Filename, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append))
	{
		return true;
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not save to file %s"), *Filename);
		return false;
	}
}

// FString UVisionBPLib::FormatFrameFilename(FString FilenameFmt)
// {
// 	int FrameNumber = GFrameNumber;
// 	return FString::Printf(*FilenameFmt, FrameNumber);
// }

bool UVisionBPLib::SendMessageBP(const FString& Message)
{
	FUnrealcvServer::Get().TcpServer->SendMessage(Message);
	return true;
}

void UVisionBPLib::SavePng(const TArray<FColor>& InImageData, int Width, int Height, FString Filename, bool bKeepAlpha)
{
	if (InImageData.Num() == 0 || Width == 0 || Height == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UVisionBPLib::SavePng - The input image is empty."));
		UE_LOG(LogTemp, Warning, TEXT("Num of pixels: %d, width: %d, height: %d"), InImageData.Num(), Width, Height);
		return;
	}
	FImageUtil ImageUtil;
	if (!bKeepAlpha)
	// Get rid of alpha which will accidentally make the image invisible
	{
		// TODO: This is slow
		TArray<FColor> ImageData = InImageData;
		for (FColor& Pixel : ImageData)
		{
			Pixel.A = 255;
		}
		ImageUtil.SavePngFile(ImageData, Width, Height, Filename);
	}
	else
	{
		ImageUtil.SavePngFile(InImageData, Width, Height, Filename);
	}
}

void UVisionBPLib::SaveNpy(const TArray<float>& FloatData, int Width, int Height, FString Filename)
{
	FImageUtil ImageUtil;
	TArray<uint8> BinaryData = FSerializationUtils::Array2Npy(FloatData, Width, Height, 1);
	ImageUtil.SaveFile(BinaryData, Filename);
}

void UVisionBPLib::GetBoneTransform(
	const USkeletalMeshComponent* SkeletalMeshComponent,
	const TArray<FString>& IncludedBones,
	TArray<FString>& BoneNames,
	TArray<FTransform>& BoneTransforms,
	bool bWorldSpace
)
{
	FBoneSensor BoneSensor(SkeletalMeshComponent);
	BoneSensor.SetBones(IncludedBones);

	TArray<FBoneInfo> BonesInfo = BoneSensor.GetBonesInfo();
	for (FBoneInfo BoneInfo : BonesInfo)
	{
		BoneNames.Add(BoneInfo.BoneName);
		if (bWorldSpace)
		{
			BoneTransforms.Add(BoneInfo.WorldTM);
		}
		else
		{
			BoneTransforms.Add(BoneInfo.ComponentTM);
		}
	}
}

void UVisionBPLib::GetBoneTransformJson(
	const USkeletalMeshComponent* SkeletalMeshComponent,
	const TArray<FString>& IncludedBones,
	TArray<FString>& BoneNames,
	TArray<FJsonObjectBP>& BoneTransformsJson,
	bool bWorldSpace
)
{
	TArray<FTransform> BoneTransforms;
	GetBoneTransform(
		SkeletalMeshComponent, 
		IncludedBones, 
		BoneNames, 
		BoneTransforms,
		bWorldSpace
	);
	for (FTransform& Transform : BoneTransforms)
	{
		FJsonObjectBP JsonObjectBP(Transform);
		BoneTransformsJson.Add(JsonObjectBP);
	}
}

void UVisionBPLib::GetVertexArray(const AActor* Actor, TArray<FVector>& VertexArray)
{
	FVertexSensor VertexSensor(Actor);
	VertexArray = VertexSensor.GetVertexArray();
}

void UVisionBPLib::UpdateInput(APawn* Pawn, bool Enable)
{
	if (!IsValid(Pawn))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not update Pawn input, invalid input"));
		return;
	}
	UWorld* World = FUnrealcvServer::Get().GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	check(PlayerController);
	if (Enable)
	{
		UE_LOG(LogUnrealCV, Log, TEXT("Enabling input"));
		Pawn->EnableInput(PlayerController);
	}
	else
	{
		UE_LOG(LogUnrealCV, Log, TEXT("Disabling input"));
		Pawn->DisableInput(PlayerController);
	}
}

void UVisionBPLib::GetActorList(TArray<AActor*>& ActorList)
{
	TArray<UObject*> UObjectList;
	bool bIncludeDerivedClasses = true;
	EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject;
	EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::AllFlags;
	GetObjectsOfClass(AActor::StaticClass(), UObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);

	for (UObject* ActorObject : UObjectList)
	{
		AActor* Actor = Cast<AActor>(ActorObject);
		if (Actor->GetWorld() != FUnrealcvServer::Get().GetWorld()) continue;
		ActorList.AddUnique(Actor);
	}
}

void UVisionBPLib::GetAnnotationColor(AActor* Actor, FColor& AnnotationColor)
{
	AUnrealcvWorldController* WorldController = FUnrealcvServer::Get().WorldController.Get();
	if (IsValid(WorldController))
	{
		WorldController->ObjectAnnotator.GetAnnotationColor(Actor, AnnotationColor);
	}
}

/*
void UVisionBPLib::AnnotateWorld()
{
	AUnrealcvWorldController* WorldController = FUnrealcvServer::Get().WorldController.Get();
	if (IsValid(WorldController))
	{
		WorldController->ObjectAnnotator.AnnotateWorld(FUnrealcvServer::Get().GetWorld());
	}
}
*/

TArray<FVector> UVisionBPLib::SkinnedMeshComponentGetVertexArray(USkinnedMeshComponent* Component)
{
	TArray<FVector> VertexArray;
	if (!IsValid(Component)) return VertexArray;

#if ENGINE_MINOR_VERSION < 19
	Component->ComputeSkinnedPositions(VertexArray);
#else
	// Ref: https://github.com/EpicGames/UnrealEngine/blob/4.19/Engine/Source/Runtime/Engine/Private/PhysicsEngine/PhysAnim.cpp#L671
	if (Component->MeshObject == nullptr)
	{
		return VertexArray;
	}

	TArray<FMatrix> RefToLocals;
	FSkinWeightVertexBuffer& SkinWeightBuffer = *Component->GetSkinWeightBuffer(0);
	const FSkeletalMeshLODRenderData& LODData = Component->MeshObject->GetSkeletalMeshRenderData().LODRenderData[0];
	Component->CacheRefToLocalMatrices(RefToLocals);
	USkinnedMeshComponent::ComputeSkinnedPositions(Component, VertexArray, RefToLocals, LODData, SkinWeightBuffer);
#endif

	return VertexArray;
	// SkinnedMeshComponent.cpp::ComputeSkinnedPositions
	// for (int VertexIndex = 0; VertexIndex < ; VertexIndex++)
	// {
	// 	Component->GetSkinnedVertexPosition(VertexIndex);
	// }
	// USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	// if (!IsValid(SkeletalMesh)) return;
	//
	// TIndirectArray<FStaticLODModel>& LODModels = SkeletalMesh->GetResourceForRendering()->LODModels;
	//
	// uint32 NumLODLevel = LODModels.Num();
	//
	// for (uint32 LODIndex = 0; LODIndex < NumLODLevel; LODIndex++)
	// {
	// }
}

// Make sure the make the Mesh CPU accessible, otherwise the vertex location can not be read from the packaged game
TArray<FVector> UVisionBPLib::StaticMeshComponentGetVertexArray(UStaticMeshComponent* StaticMeshComponent)
{
	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
	TArray<FVector> VertexArray;

	if (StaticMesh)
	{
		uint32 NumLODLevel = StaticMesh->RenderData->LODResources.Num();
		for (uint32 LODIndex = 0; LODIndex < NumLODLevel; LODIndex++)
		{
			FStaticMeshLODResources& LODModel = StaticMesh->RenderData->LODResources[LODIndex];
			FStaticMeshComponentLODInfo* InstanceMeshLODInfo = NULL;

			uint32 NumVertices = LODModel.GetNumVertices();

#if ENGINE_MINOR_VERSION < 19
			FPositionVertexBuffer& PositionVertexBuffer = LODModel.PositionVertexBuffer;
#else
			FStaticMeshVertexBuffers& StaticMeshVertexBuffers = LODModel.VertexBuffers;
			FPositionVertexBuffer& PositionVertexBuffer = StaticMeshVertexBuffers.PositionVertexBuffer;
#endif

			for (uint32 VertexIndex = 0; VertexIndex < PositionVertexBuffer.GetNumVertices(); VertexIndex++)
			{
				FVector Position = PositionVertexBuffer.VertexPosition(VertexIndex);
				VertexArray.Add(Position);
			}

			//FStaticMeshVertexBuffer& VertexBuffer = LODModel.VertexBuffer;
			//const uint8* Data = VertexBuffer.GetRawVertexData();

			//FRawStaticIndexBuffer& IndexBuffer = LODModel.IndexBuffer;
			//FIndexArrayView IndexArrayView = IndexBuffer.GetArrayView();
		}
	}

	return VertexArray;
}

TArray<FVector> UVisionBPLib::GetVertexArrayFromMeshComponent(UMeshComponent* MeshComponent)
{
	TArray<FVector> VertexArray;
	if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
	{
		VertexArray = UVisionBPLib::StaticMeshComponentGetVertexArray(StaticMeshComponent);
	}
	if (USkinnedMeshComponent* SkinnedMeshComponent = Cast<USkinnedMeshComponent>(MeshComponent))
	{
		VertexArray = UVisionBPLib::SkinnedMeshComponentGetVertexArray(SkinnedMeshComponent);
	}
	return VertexArray;
}

UFusionCamSensor* UVisionBPLib::GetPlayerSensor()
{
	APawn* Pawn = FUnrealcvServer::Get().GetPawn();
	if (!IsValid(Pawn)) return nullptr;

	UActorComponent* ActorComponent = Pawn->GetComponentByClass(UFusionCamSensor::StaticClass());
	UFusionCamSensor* CamSensor = Cast<UFusionCamSensor>(ActorComponent);
	return CamSensor;
}

void UVisionBPLib::AnnotateWorld()
{
	FUnrealcvServer::Get().WorldController->ObjectAnnotator.AnnotateWorld(GWorld);
}