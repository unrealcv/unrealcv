// Weichao Qiu @ 2018
#include "UnrealCVPrivate.h"
#include "AnimationBP.h"

FString UAnimationBP::GetAnimationName(USkeletalMeshComponent* SkeletalMeshComponent)
{
	FString AnimationName;
	return AnimationName;
}

// From: https://answers.unrealengine.com/questions/263204/getting-the-animations-list-from-persona.html
TArray<UAnimationAsset *> UAnimationBP::GetAllAnimationOfSkeleton(USkeleton *Skeleton)
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

TArray<UAnimSequence *> UAnimationBP::GetAllAnimationSequenceOfSkeleton(USkeleton *Skeleton)
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

// void UAnimationBP::PlayAnimationToEnd(
// 	USkeletalMeshComponent* SkeletalMeshComponent, 
// 	UAnimationAsset* AnimationAsset)
// {
	// SkeletalMeshComponent->PlayAnimation(AnimationAsset);
	// while (SkeletalMeshComponent->IsPlaying())
	// {
	// }
	// // Wait until the animation finished
// }