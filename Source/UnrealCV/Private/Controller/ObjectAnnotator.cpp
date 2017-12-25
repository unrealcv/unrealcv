// Weichao Qiu @ 2017

#include "UnrealCVPrivate.h"
#include "ObjectAnnotator.h"

FColor GetColorFromColorMap(int32 ObjectIndex);

FObjectAnnotator::FObjectAnnotator()
{

}

/** Annotate all static mesh in the world */
void FObjectAnnotator::AnnotateStaticMesh()
{
	for (TActorIterator<AActor> ActorItr(FUE4CVServer::Get().GetGameWorld()); ActorItr; ++ActorItr)
	{
		AActor *Actor = *ActorItr;
		FColor AnnotationColor = GetDefaultColor(Actor);

		// Use VertexColor as annotation
		FVertexColorPainter::PaintVertexColor(Actor, AnnotationColor);

		TArray<UActorComponent*> AnnotationComponents = Actor->GetComponentsByClass(UAnnotationComponent::StaticClass());
		if (AnnotationComponents.Num() != 0)
		{
			continue; // Do not overwrite already annotated object
		}

		// Use AnnotationComponent as annotation
		TArray<UActorComponent*> StaticMeshComponents = Actor->GetComponentsByClass(UStaticMeshComponent::StaticClass());
		for (UActorComponent* Component : StaticMeshComponents)
		{
			UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
			UAnnotationComponent* AnnotationComponent = NewObject<UAnnotationComponent>(StaticMeshComponent);
			AnnotationComponent->AnnotationColor = AnnotationColor;
			AnnotationComponent->SetupAttachment(StaticMeshComponent);
			AnnotationComponent->RegisterComponent();
		}
	}
}

FColor FObjectAnnotator::GetDefaultColor(AActor* Actor)
{
	TArray<FString> AnnotatedObjects;
	AnnotationColors.GetKeys(AnnotatedObjects);

	FString ActorName = Actor->GetName();
	if (AnnotatedObjects.Contains(ActorName))
	{
		// Already initialized
		return AnnotationColors[ActorName];
	}

	int ColorIndex = AnnotatedObjects.Num();
	FColor AnnotationColor = GetColorFromColorMap(ColorIndex);

	// CHECK: Add the annotation color regardless successful or not
	AnnotationColors.Emplace(ActorName, AnnotationColor);
	return AnnotationColor;
}

bool FObjectAnnotator::SetAnnotationColor(AActor* Actor, const FColor& AnnotationColor)
{
	TArray<UActorComponent*> AnnotationComponents = Actor->GetComponentsByClass(UAnnotationComponent::StaticClass());
	for (UActorComponent* Component : AnnotationComponents)
	{
		UAnnotationComponent* AnnotationComponent = Cast<UAnnotationComponent>(Component);
		AnnotationComponent->AnnotationColor = AnnotationColor;
		AnnotationComponent->MarkRenderStateDirty();
	}
	return true;
}

bool FObjectAnnotator::GetAnnotationColor(AActor* Actor, FColor& AnnotationColor)
{
	TArray<UActorComponent*> AnnotationComponents = Actor->GetComponentsByClass(UAnnotationComponent::StaticClass());
	for (UActorComponent* Component : AnnotationComponents)
	{
		UAnnotationComponent* AnnotationComponent = Cast<UAnnotationComponent>(Component);
		AnnotationColor = AnnotationComponent->AnnotationColor;
		return true;
		// Assume all annotation components of an actor has the same color
	}
	return false;
}

bool FObjectAnnotator::SetVertexColor(AActor* Actor, const FColor& AnnotationColor)
{
	FVertexColorPainter::PaintVertexColor(Actor, AnnotationColor);
	return true;
}


bool FObjectAnnotator::GetVertexColor(AActor* Actor, FColor& AnnotationColor)
{
	return true;
}

void FObjectAnnotator::SaveAnnotation(FString JsonFilename)
{
	// TODO: not implemented yet
}

void FObjectAnnotator::LoadAnnotation(FString JsonFilename)
{
	// TODO: not implemented yet
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
