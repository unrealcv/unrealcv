// Weichao Qiu @ 2017
#pragma once

#include "CoreMinimal.h"

class UNREALCV_API FSerializationUtils
{
public:
	static TArray<uint8> Array2Npy(const TArray<FFloat16Color>& ImageData, int32 Width, int32 Height, int32 Channel);
	static TArray<uint8> Array2Npy(const TArray<float>& ImageData, int32 Width, int32 Height, int32 Channel);
	static TArray<uint8> Image2Npy(const TArray<FColor>& ImageData, int32 Width, int32 Height, int32 Channel);
	static TArray64<uint8> Image2Png(const TArray<FColor>& Image, int32 Width, int32 Height);
	static TArray64<uint8> Image2Exr(const TArray<FFloat16Color>& FloatImage, int Width, int Height);
	static FString VertexList2Obj(const TArray<FVector>& VertexList);
};
