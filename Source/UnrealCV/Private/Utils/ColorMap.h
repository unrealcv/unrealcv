#if 0 // TODO: Remove this file
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
				uint8 R = GetChannelValue(Fix1 ? MaxVal : I);
				uint8 G = GetChannelValue(Fix2 ? MaxVal : J);
				uint8 B = GetChannelValue(Fix3 ? MaxVal : K);
				FColor Color(R, G, B, 255);
				ColorMap.Add(Color);
			}
		}
	}
}

/** TODO: support more than 1000 objects */
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

#endif

