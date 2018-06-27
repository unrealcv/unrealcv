// Weichao Qiu @ 2018
#include "AnimationBPLib.h"
#include "Runtime/CoreUObject/Public/UObject/UObjectIterator.h"
#include "Runtime/Engine/Classes/Animation/AnimationAsset.h"
#include "Runtime/Engine/Classes/Animation/Skeleton.h"
#include "Runtime/Engine/Classes/Animation/AnimSequence.h"

FString UAnimationBPLib::GetAnimationName(USkeletalMeshComponent* SkeletalMeshComponent)
{
	FString AnimationName;
	return AnimationName;
}

// From: https://answers.unrealengine.com/questions/263204/getting-the-animations-list-from-persona.html
TArray<UAnimationAsset *> UAnimationBPLib::GetAllAnimationOfSkeleton(USkeleton *Skeleton)
{
	TArray<UAnimationAsset *> AnimsArray;

	if (Skeleton == NULL)
	{
		return AnimsArray;
	}

	for (TObjectIterator<UAnimationAsset> Itr; Itr; ++Itr)
	{
		if (Skeleton->GetWorld() != (*Itr)->GetWorld())
		{
			continue;
		}

		if (Skeleton == (*Itr)->GetSkeleton())
		{
			AnimsArray.Add(*Itr);
		}
	}
	return AnimsArray;
}

TArray<UAnimSequence *> UAnimationBPLib::GetAllAnimationSequenceOfSkeleton(USkeleton *Skeleton)
{
	TArray<UAnimSequence*> AnimsArray;

	if (Skeleton == NULL)
	{
		return AnimsArray;
	}

	for (TObjectIterator<UAnimSequence> Itr; Itr; ++Itr)
	{
		if (Skeleton->GetWorld() != (*Itr)->GetWorld())
		{
			continue;
		}

		if (Skeleton == (*Itr)->GetSkeleton())
		{
			AnimsArray.Add(*Itr);
		}
	}
	return AnimsArray;
}

// void UAnimationBPLib::PlayAnimationToEnd(
// 	USkeletalMeshComponent* SkeletalMeshComponent, 
// 	UAnimationAsset* AnimationAsset)
// {
	// SkeletalMeshComponent->PlayAnimation(AnimationAsset);
	// while (SkeletalMeshComponent->IsPlaying())
	// {
	// }
	// // Wait until the animation finished
// }