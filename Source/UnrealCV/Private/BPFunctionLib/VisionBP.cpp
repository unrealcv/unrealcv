// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "VisionBP.h"
#include "ImageUtil.h"
#include "UE4CVServer.h"
#include "Serialization.h"
#include "VertexSensor.h"
#include "BoneSensor.h"

// FString UVisionBP::SerializeBoneInfo(USkeletalMeshComponent* Component)
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

bool UVisionBP::CreateFile(const FString& Filename)
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

bool UVisionBP::SaveData(const FString& Data, const FString& Filename)
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

bool UVisionBP::AppendData(const FString& Data, const FString& Filename)
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

// FString UVisionBP::FormatFrameFilename(FString FilenameFmt)
// {
// 	int FrameNumber = GFrameNumber;
// 	return FString::Printf(*FilenameFmt, FrameNumber);
// }

bool UVisionBP::SendMessageBP(const FString& Message)
{
	FUE4CVServer::Get().NetworkManager->SendMessage(Message);
	return true;
}

void UVisionBP::SavePng(const TArray<FColor>& InImageData, int Width, int Height, FString Filename, bool bKeepAlpha)
{
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

void UVisionBP::SaveNpy(const TArray<float>& FloatData, int Width, int Height, FString Filename)
{
	FImageUtil ImageUtil;
	TArray<uint8> BinaryData = FSerializationUtils::Array2Npy(FloatData, Width, Height, 1);
	ImageUtil.SaveFile(BinaryData, Filename);
}

void UVisionBP::GetBoneTransform(
	const USkeletalMeshComponent* SkeletalMeshComponent,
	const TArray<FString>& IncludedBones,
	TArray<FString>& BoneNames,
	TArray<FTransform>& BoneTransform,
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
			BoneTransform.Add(BoneInfo.WorldTM);
		}
		else
		{
			BoneTransform.Add(BoneInfo.ComponentTM);
		}
	}
}

void UVisionBP::GetVertexArray(const AActor* Actor, TArray<FVector>& VertexArray)
{
	FVertexSensor VertexSensor(Actor);
	VertexArray = VertexSensor.GetVertexArray();
}

void UVisionBP::UpdateInput(APawn* Pawn, bool Enable)
{
	if (!IsValid(Pawn))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not update Pawn input, invalid input"));
		return;
	}
	UWorld* World = FUE4CVServer::Get().GetGameWorld();
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

void UVisionBP::GetActorList(TArray<AActor*>& ActorList)
{
	TArray<UObject*> UObjectList;
	bool bIncludeDerivedClasses = true;
	EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject;
	EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::AllFlags;
	GetObjectsOfClass(AActor::StaticClass(), UObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);

	for (UObject* ActorObject : UObjectList)
	{
		AActor* Actor = Cast<AActor>(ActorObject);
		if (Actor->GetWorld() != FUE4CVServer::Get().GetGameWorld()) continue;
		ActorList.AddUnique(Actor);
	}
}

void UVisionBP::GetAnnotationColor(AActor* Actor, FColor& AnnotationColor)
{
	FObjectAnnotator& Annotator = FUE4CVServer::Get().WorldController->ObjectAnnotator;
	Annotator.GetAnnotationColor(Actor, AnnotationColor);
}

void UVisionBP::AnnotateWorld()
{
	FUE4CVServer::Get().WorldController->ObjectAnnotator.AnnotateWorld(FUE4CVServer::Get().GetGameWorld());
}

UFusionCamSensor* UVisionBP::GetPlayerSensor()
{
	APawn* Pawn = FUE4CVServer::Get().GetPawn();
	if (!IsValid(Pawn)) return nullptr;

	UActorComponent* ActorComponent = Pawn->GetComponentByClass(UFusionCamSensor::StaticClass());
	UFusionCamSensor* CamSensor = Cast<UFusionCamSensor>(ActorComponent);
	return CamSensor;
}