// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCVPrivate.h"
#include "BoneVisualizationHUD.h"

void ABoneVisualizationHUD::PostRender()
{
	if (this->Canvas)
	{
		bool bSimpleBones = true;
		for (TObjectIterator<USkeletalMeshComponent> Iter; Iter; ++Iter)
		{
			USkeletalMeshComponent* Component = *Iter;
			if (Component->GetWorld() == this->GetWorld()) // Make sure only use the components of this world!
			{
				Component->DebugDrawBones(Canvas, bSimpleBones);
			}
		}
	}

	for (TObjectIterator<USkeletalMeshComponent> Iter; Iter; ++Iter)
	{
		USkeletalMeshComponent* Component = *Iter;
		if (Component->GetWorld() != this->GetWorld()) continue; // Make sure only use the components of this world!

																 // Export bones from the SkeletalMeshComponent
		auto RequiredBones = Component->RequiredBones;
		auto SkeletalMesh = Component->SkeletalMesh;
		auto ComponentSpaceTransforms = Component->GetComponentSpaceTransforms();
		auto ComponentToWorld = Component->ComponentToWorld;

		for (int32 Index = 0; Index < RequiredBones.Num(); ++Index)
		{
			int32 BoneIndex = RequiredBones[Index];
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
			UE_LOG(LogTemp, Log, TEXT("Bone: %d, Start: %s, End: %s"), BoneIndex, *Start.ToString(), *End.ToString());
		}
	}
}