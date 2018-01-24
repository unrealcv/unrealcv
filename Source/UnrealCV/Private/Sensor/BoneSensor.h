// Weichao Qiu @ 2017
#pragma once

#include "Engine.h"

struct FBoneInfo
{
	FString BoneName;
	FTransform BoneTM;
	FTransform ComponentTM;
	FTransform WorldTM;

	FString ToString()
	{
		return FString::Printf(TEXT("Bone: %s, %s"), *BoneName, *BoneTM.ToHumanReadableString());
	}
};

/** A native BoneSensor, use VisionBP to involve in blueprint */
class FBoneSensor
{
public:
	FBoneSensor(const class USkeletalMeshComponent* InSkeletalMeshComponent);

	void SetBones(const TArray<FString>& InIncludedBoneNames);

	TArray<FBoneInfo> GetBonesInfo();

private:
	const USkeletalMeshComponent* Component;

	/** Only track the bone information specified in this list */
	TArray<FString> IncludedBoneNames;
};
