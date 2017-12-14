// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "VisionBP.h"

FString UVisionBP::SerializeBoneInfo(USkeletalMeshComponent* Component)
{
	FString Data;
	TArray<FBoneIndexType> RequiredBones = Component->RequiredBones;
	USkeletalMesh* SkeletalMesh = Component->SkeletalMesh;
	auto ComponentSpaceTransforms = Component->GetComponentSpaceTransforms();
	FTransform ComponentToWorld = Component->ComponentToWorld;

	for (int32 Index = 0; Index < RequiredBones.Num(); ++Index)
	{
		int32 BoneIndex = RequiredBones[Index];
		// FReferenceSkeleton
		int32 ParentIndex = SkeletalMesh->RefSkeleton.GetParentIndex(BoneIndex);

		FVector Start, End;
		FTransform BoneTM = ComponentSpaceTransforms[BoneIndex] * ComponentToWorld;

		End = BoneTM.GetLocation();
		if (ParentIndex >= 0)
		{
			Start = (ComponentSpaceTransforms[ParentIndex] * ComponentToWorld).GetLocation();
		}
		else // No parent??
		{
			Start = ComponentToWorld.GetLocation();
		}
		FString BoneName = SkeletalMesh->RefSkeleton.GetBoneName(BoneIndex).ToString();
		UE_LOG(LogTemp, Log, TEXT("Bone: %d, %s, Start: %s, End: %s"), BoneIndex, *BoneName, *Start.ToString(), *End.ToString());
		Data += FString::Printf(TEXT("Bone: %d, %s, Start: %s, End: %s\r\n"), BoneIndex, *BoneName, *Start.ToString(), *End.ToString());
	}

	return Data;
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

FString UVisionBP::FormatFrameFilename(FString FilenameFmt)
{
	int FrameNumber = GFrameNumber;
	return FString::Printf(*FilenameFmt, FrameNumber);
}

bool UVisionBP::SendMessageBP(const FString& Message)
{
	FUE4CVServer::Get().NetworkManager->SendMessage(Message);
	return true;
}
