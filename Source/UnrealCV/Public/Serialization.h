#pragma once

class SerializationUtils
{
public:
	static TArray<uint8> Array2Npy(const TArray<FFloat16Color>& ImageData, int32 Width, int32 Height, int32 Channel);
	static TArray64<uint8> Image2Png(const TArray<FColor>& Image, int32 Width, int32 Height);
	static TArray64<uint8> Image2Exr(const TArray<FFloat16Color>& FloatImage, int Width, int Height);
};
