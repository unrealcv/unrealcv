#include "UnrealCVPrivate.h"
#include "ObjectPainter.h"
#include "StaticMeshResources.h"
#include "UE4CVServer.h"
#include "SceneViewport.h"

static TMap<uint8, uint8> DecodeColorValue; // Convert Encoded Color to Display Color, for numerical issue

// Utility function to generate color map
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
			for (uint32 Value = Step-1; Value <= 256; Value += Step * 2)
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
	for (int32 I = 0; I <= (Fix1 ? 0 : MaxVal-1); I++)
	{
		for (int32 J = 0; J <= (Fix2 ? 0 : MaxVal-1); J++)
		{
			for (int32 K = 0; K <= (Fix3 ? 0 : MaxVal-1); K++)
			{
				uint8 R = GetChannelValue(Fix1 ? MaxVal : I);
				uint8 G = GetChannelValue(Fix2 ? MaxVal : J);
				uint8 B = GetChannelValue(Fix3 ? MaxVal : K);
				FColor Color(R, G, B, 255);
				ColorMap.Add(Color);
			}
		}
	}
}

FColor GetColorFromColorMap(int32 ObjectIndex)
{
	static TArray<FColor> ColorMap;
	if (ColorMap.Num() == 0)
	{
		for (int32 MaxChannelIndex = 0; MaxChannelIndex < 10; MaxChannelIndex++) // Get color map for 1000 objects
		{
			// GetColors(MaxChannelIndex, false, false, false, ColorMap);
			GetColors(MaxChannelIndex, false, false, true , ColorMap);
			GetColors(MaxChannelIndex, false, true , false, ColorMap);
			GetColors(MaxChannelIndex, false, true , true , ColorMap);
			GetColors(MaxChannelIndex, true , false, false, ColorMap);
			GetColors(MaxChannelIndex, true , false, true , ColorMap);
			GetColors(MaxChannelIndex, true , true , false, ColorMap);
			GetColors(MaxChannelIndex, true , true , true , ColorMap);
		}
	}
	check(ColorMap.Num() == 1000);
	check(ObjectIndex >= 0 && ObjectIndex <= 1000);
	return ColorMap[ObjectIndex];
}

FObjectPainter& FObjectPainter::Get()
{
	static FObjectPainter Singleton(NULL);
	return Singleton;
}

FObjectPainter::FObjectPainter(ULevel* InLevel)
{
	this->Level = InLevel;
}

FExecStatus FObjectPainter::SetActorColor(FString ObjectName, FColor Color)
{
	if (ObjectMap.Contains(ObjectName))
	{
		AActor* Actor = ObjectMap[ObjectName];
		if (PaintObject(Actor, Color))
		{
			ObjectColorMap.Emplace(ObjectName, Color);
			return FExecStatus::OK();
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Failed to paint object %s"), *ObjectName));
		}
	}
	else
	{
		return FExecStatus::Error(FString::Printf(TEXT("Object %s not exist"), *ObjectName));
	}
}

/** Check whether an actor can be painted with vertex color */
bool IsPaintable(AActor* Actor)
{
	TArray<UMeshComponent*> PaintableComponents;
	Actor->GetComponents<UMeshComponent>(PaintableComponents);
	if (PaintableComponents.Num() == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

FExecStatus FObjectPainter::GetActorColor(FString ObjectName)
{
	if (ObjectColorMap.Contains(ObjectName))
	{
		FColor ObjectColor = ObjectColorMap[ObjectName]; // Make sure the object exist
		FString Message = ObjectColor.ToString();
		// FString Message = "%.3f %.3f %.3f %.3f";
		return FExecStatus::OK(Message);
	}
	else
	{
		return FExecStatus::Error(FString::Printf(TEXT("Object %s not exist"), *ObjectName));
	}
}

// This should be moved to command handler
FExecStatus FObjectPainter::GetObjectList()
{
	TArray<FString> Keys;
	this->ObjectMap.GetKeys(Keys);
	FString Message = "";
	for (auto ObjectName : Keys)
	{
		Message += ObjectName + " ";
	}
	Message = Message.LeftChop(1);
	return FExecStatus::OK(Message);
}

TMap<FString, AActor*>& FObjectPainter::GetObjectMap()
{
	// This list needs to be generated everytime the game restarted.
	check(Level);
	for (AActor* Actor : Level->Actors)
	{
		if (Actor && IsPaintable(Actor)) 
		{
			FString ActorLabel = Actor->GetHumanReadableName();
			ObjectMap.Emplace(ActorLabel, Actor);
		}
	}
	return ObjectMap;
}

bool FObjectPainter::PaintColors()
{
	FSceneViewport* SceneViewport = GWorld->GetGameViewport()->GetGameViewport();

	check(Level);
	uint32 ObjectIndex = 0;

	TArray<AActor*> Actors;
	ObjectMap.GenerateValueArray(Actors);
	for (auto Actor : Actors)
	{
		FString ActorLabel = Actor->GetHumanReadableName(); // GetActorLabel can not work
		FColor NewColor = GetColorFromColorMap(ObjectIndex);

		ObjectColorMap.Emplace(ActorLabel, NewColor);
		check(PaintObject(Actor, NewColor));
		ObjectIndex++;
	}
	return true;
}

/** DisplayColor is the color that the screen will show
	If DisplayColor.R = 128, the display will show 0.5 voltage
	To achieve this, UnrealEngine will do gamma correction.
	The value on image will be 187.
	https://en.wikipedia.org/wiki/Gamma_correction#Methods_to_perform_display_gamma_correction_in_computing
*/
bool FObjectPainter::PaintObject(AActor* Actor, const FColor& Color, bool IsColorGammaEncoded)
{
	if (!Actor) return false;

	FColor NewColor;
	if (IsColorGammaEncoded)
	{
		FLinearColor LinearColor = FLinearColor::FromPow22Color(Color);
		NewColor = LinearColor.ToFColor(false);
	}
	else
	{
		NewColor = Color;
	}

	TArray<UMeshComponent*> PaintableComponents;
	Actor->GetComponents<UMeshComponent>(PaintableComponents);


	for (auto MeshComponent : PaintableComponents)
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			if (UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh)
			{
				uint32 NumLODLevel = StaticMeshComponent->StaticMesh->RenderData->LODResources.Num();
				for (uint32 PaintingMeshLODIndex = 0; PaintingMeshLODIndex < NumLODLevel; PaintingMeshLODIndex++)
				{
					FStaticMeshLODResources& LODModel = StaticMeshComponent->StaticMesh->RenderData->LODResources[PaintingMeshLODIndex];
					FStaticMeshComponentLODInfo* InstanceMeshLODInfo = NULL;

					// PaintingMeshLODIndex + 1 is the minimum requirement, enlarge if not satisfied
					StaticMeshComponent->SetLODDataCount(PaintingMeshLODIndex + 1, StaticMeshComponent->LODData.Num());
					InstanceMeshLODInfo = &StaticMeshComponent->LODData[PaintingMeshLODIndex];

					InstanceMeshLODInfo->ReleaseOverrideVertexColorsAndBlock();
					// Setup OverrideVertexColors
					// if (!InstanceMeshLODInfo->OverrideVertexColors) // TODO: Check this
					{
						InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer;

						FColor FillColor = FColor(255, 255, 255, 255);
						InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(FColor::White, LODModel.GetNumVertices());
					}

					uint32 NumVertices = LODModel.GetNumVertices();
					check(InstanceMeshLODInfo->OverrideVertexColors);
					check(NumVertices <= InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices());
					// StaticMeshComponent->CachePaintedDataIfNecessary();

					for (uint32 ColorIndex = 0; ColorIndex < NumVertices; ++ColorIndex)
					{
						// LODModel.ColorVertexBuffer.VertexColor(ColorIndex) = NewColor;  // This is vertex level
						// Need to initialize the vertex buffer first
						uint32 NumOverrideVertexColors = InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices();
						uint32 NumPaintedVertices = InstanceMeshLODInfo->PaintedVertices.Num();
						// check(NumOverrideVertexColors == NumPaintedVertices);
						InstanceMeshLODInfo->OverrideVertexColors->VertexColor(ColorIndex) = NewColor;
						// InstanceMeshLODInfo->PaintedVertices[ColorIndex].Color = NewColor;
					}
					BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);
					StaticMeshComponent->MarkRenderStateDirty();
					// BeginUpdateResourceRHI(InstanceMeshLODInfo->OverrideVertexColors);


					/*
					// TODO: Need to check other LOD levels
					// Use flood fill to paint mesh vertices
					UE_LOG(LogUnrealCV, Warning, TEXT("%s:%s has %d vertices"), *Actor->GetActorLabel(), *StaticMeshComponent->GetName(), NumVertices);

					if (LODModel.ColorVertexBuffer.GetNumVertices() == 0)
					{
						// Mesh doesn't have a color vertex buffer yet!  We'll create one now.
						LODModel.ColorVertexBuffer.InitFromSingleColor(FColor(255, 255, 255, 255), LODModel.GetNumVertices());
					}

					*/
				}
			}
		}
	}
	return true;
}

AActor* FObjectPainter::GetObject(FString ObjectName)
{
	/** Return the pointer of an object, return NULL if object not found */
	if (ObjectMap.Contains(ObjectName))
	{
		return ObjectMap[ObjectName];
	}
	else
	{
		return NULL;
	}
}

void FObjectPainter::Reset(ULevel* InLevel)
{
	this->Level = InLevel;
	this->ObjectColorMap.Empty();
	this->ObjectMap.Empty();
	this->GetObjectMap();
}