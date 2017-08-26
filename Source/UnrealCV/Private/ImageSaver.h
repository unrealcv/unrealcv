#pragma once
#include "Engine.h"

class FImageSaver
{
public:
    static bool SavePng(const TArray<FColor>& Image, int Width, int Height, FString Filename);

    static bool SaveExr(const TArray<FFloat16Color>& FloatImage, int Width, int Height, FString Filename);

    /** Save the image to a file according to its file extension */
    static bool SaveFile(const TArray<FColor>& Image, int Width, int Height, FString Filename);

    static bool SaveFile(const TArray<FFloat16Color>& FloatImage, int Width, int Height, FString Filename);
};
