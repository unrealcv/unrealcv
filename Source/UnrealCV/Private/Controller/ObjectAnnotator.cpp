// Weichao Qiu @ 2017

#include "UnrealCVPrivate.h"
#include "ObjectAnnotator.h"

FColor GetColorFromColorMap(int32 ObjectIndex);

FObjectAnnotator::FObjectAnnotator()
{
}



/** Annotate all static mesh in the world */
void FObjectAnnotator::AnnotateWorld(UWorld* World)
{
	if (!IsValid(World))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not annotate world, the world is not valid"));
		return;
	}

	TArray<AActor*> ActorArray;
	GetAnnotableActors(World, ActorArray);

	for (AActor* Actor : ActorArray)
	{
		FColor AnnotationColor = GetDefaultColor(Actor);

		if (!IsValid(Actor))
		{
			UE_LOG(LogUnrealCV, Warning, TEXT("Found invalid actor in AnnotateWorld"));
			continue;
		}
		// Use VertexColor as annotation
		this->SetAnnotationColor(Actor, AnnotationColor);
	}
	UE_LOG(LogUnrealCV, Log, TEXT("Annotate mesh of the scene (%d)"), AnnotationColors.Num());
}


void FObjectAnnotator::SetAnnotationColor(AActor* Actor, const FColor& AnnotationColor)
{
	if (!IsValid(Actor))
	{
		return;
	}
	// CHECK: Add the annotation color regardless successful or not
	// this->PaintVertexColor(Actor, AnnotationColor);
	this->CreateAnnotationComponent(Actor, AnnotationColor);
	this->AnnotationColors.Emplace(Actor->GetName(), AnnotationColor);
}

void FObjectAnnotator::GetAnnotationColor(AActor* Actor, FColor& AnnotationColor)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("InActor is invalid in GetAnnotationColor"));
		return;
	}

	FString ActorName = Actor->GetName();
	if (!this->AnnotationColors.Contains(ActorName))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not find actor %s in GetAnnotationColor"), *ActorName);
		return;
	}
	AnnotationColor = this->AnnotationColors[ActorName];
}

void FObjectAnnotator::GetAnnotableActors(UWorld* World, TArray<AActor*>& ActorArray)
{
	if (!IsValid(World))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The world is invalid in GetAnnotableActors"));
		return;
	}

	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor *Actor = *ActorItr;
		ActorArray.Add(Actor);
	}
}

#if ENGINE_MINOR_VERSION < 17
void FObjectAnnotator::PaintVertexColor(AActor* Actor, const FColor& AnnotationColor)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Invalid actor in PaintVertexColor"));
		return;
	}
	FVertexColorPainter::PaintVertexColor(Actor, AnnotationColor);
}
#endif

void FObjectAnnotator::CreateAnnotationComponent(AActor* Actor, const FColor& AnnotationColor)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Invalid actor in CreateAnnotationComponent"));
		return;
	}
	TArray<UActorComponent*> AnnotationComponents = Actor->GetComponentsByClass(UAnnotationComponent::StaticClass());
	if (AnnotationComponents.Num() != 0)
	{
		UE_LOG(LogUnrealCV, Log, TEXT("Skip annotated actor %d"), *Actor->GetName());
		return;
	}

	TArray<UActorComponent*> MeshComponents = Actor->GetComponentsByClass(UMeshComponent::StaticClass());
	for (UActorComponent* Component : MeshComponents)
	{
		UMeshComponent* MeshComponent = Cast<UMeshComponent>(Component);

		UAnnotationComponent* AnnotationComponent = NewObject<UAnnotationComponent>(MeshComponent);
		AnnotationComponent->AnnotationColor = AnnotationColor;
		AnnotationComponent->SetupAttachment(MeshComponent);
		AnnotationComponent->RegisterComponent();
		AnnotationComponent->MarkRenderStateDirty();
	}
}

void FObjectAnnotator::UpdateAnnotationComponent(AActor* Actor, const FColor& AnnotationColor)
{
	if (!IsValid(Actor))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Invalid actor in CreateAnnotationComponent"));
		return;
	}
	TArray<UActorComponent*> AnnotationComponents = Actor->GetComponentsByClass(UAnnotationComponent::StaticClass());
	for (UActorComponent* Component : AnnotationComponents)
	{
		UAnnotationComponent* AnnotationComponent = Cast<UAnnotationComponent>(Component);
		AnnotationComponent->AnnotationColor = AnnotationColor;
		AnnotationComponent->MarkRenderStateDirty();
	}
}



FColor FObjectAnnotator::GetDefaultColor(AActor* Actor)
{
	FString ActorName = Actor->GetName();
	if (AnnotationColors.Contains(ActorName))
	{
		// Already initialized
		return AnnotationColors[ActorName];
	}

	int ColorIndex = AnnotationColors.Num();
	FColor AnnotationColor = GetColorFromColorMap(ColorIndex);

	return AnnotationColor;
}

/** Utility function to generate color map */
int32 GetChannelValue(uint32 Index)
{
	static int32 Values[256] = { 0 };
	static bool Init = false;
	if (!Init)
	{
		float Step = 256;
		uint32 Iter = 0;
		Values[0] = 0;
		while (Step >= 1)
		{
			for (uint32 Value = Step - 1; Value <= 256; Value += Step * 2)
			{
				Iter++;
				Values[Iter] = Value;
			}
			Step /= 2;
		}
		Init = true;
	}
	if (Index >= 0 && Index <= 255)
	{
		return Values[Index];
	}
	else
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Invalid channel index"));
		check(false);
		return -1;
	}
}

void GetColors(int32 MaxVal, bool Fix1, bool Fix2, bool Fix3, TArray<FColor>& ColorMap)
{
	for (int32 I = 0; I <= (Fix1 ? 0 : MaxVal - 1); I++)
	{
		for (int32 J = 0; J <= (Fix2 ? 0 : MaxVal - 1); J++)
		{
			for (int32 K = 0; K <= (Fix3 ? 0 : MaxVal - 1); K++)
			{
				uint8 R = (uint8)GetChannelValue(Fix1 ? MaxVal : I);
				uint8 G = (uint8)GetChannelValue(Fix2 ? MaxVal : J);
				uint8 B = (uint8)GetChannelValue(Fix3 ? MaxVal : K);
				FColor Color(R, G, B, 255);
				ColorMap.Add(Color);
			}
		}
	}
}

FColor GetColorFromColorMap(int32 ObjectIndex)
{
	static TArray<FColor> ColorMap;
	int NumPerChannel = 32;
	if (ColorMap.Num() == 0)
	{
		// 32 ^ 3
		for (int32 MaxChannelIndex = 0; MaxChannelIndex < NumPerChannel; MaxChannelIndex++) // Get color map for 1000 objects
		{
			// GetColors(MaxChannelIndex, false, false, false, ColorMap);
			GetColors(MaxChannelIndex, false, false, true, ColorMap);
			GetColors(MaxChannelIndex, false, true, false, ColorMap);
			GetColors(MaxChannelIndex, false, true, true, ColorMap);
			GetColors(MaxChannelIndex, true, false, false, ColorMap);
			GetColors(MaxChannelIndex, true, false, true, ColorMap);
			GetColors(MaxChannelIndex, true, true, false, ColorMap);
			GetColors(MaxChannelIndex, true, true, true, ColorMap);
		}
	}
	if (ObjectIndex < 0 || ObjectIndex >= pow(NumPerChannel, 3))
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Object index %d is out of the color map boundary [%d, %d]"), ObjectIndex, 0, pow(NumPerChannel, 3));
	}
	return ColorMap[ObjectIndex];
}
