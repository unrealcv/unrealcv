// Weichao Qiu @ 2016
#pragma once

#include "Runtime/ImageWrapper/Public/IImageWrapper.h"
#include "Runtime/ImageWrapper/Public/IImageWrapperModule.h"
#include "Runtime/Core/Public/Modules/ModuleManager.h"

class UNREALCV_API FImageUtil
{
public:
	FImageUtil()
	{
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		PngImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
		JpgImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
	}

	/** Convert FColor array to a png binary array */
	bool ConvertToPng(const TArray<FColor>& ImageData, int Width, int Height, TArray<uint8>& PngData);

	/** Convert FColor array to a jpg binary array */
	bool ConvertToJpg(const TArray<FColor>& ImageData, int Width, int Height, TArray<uint8>& JpgData);

	bool ConvertToBmp(const TArray<FColor>& ImageData, int Width, int Height, TArray<uint8>& BmpData);

	/** Save binary data to a file */
	bool SaveFile(const TArray<uint8>& BinaryData, const FString& Filename);

	bool SavePngFile(const TArray<FColor>& ImageData, int Width, int Height, const FString& Filename)
	{
		TArray<uint8> PngData;
		ConvertToPng(ImageData, Width, Height, PngData);
		SaveFile(PngData, Filename);
		return true;
	}

	bool SaveJpgFile(const TArray<FColor>& ImageData, int Width, int Height, const FString& Filename)
	{
		TArray<uint8> JpgData;
		ConvertToJpg(ImageData, Width, Height, JpgData);
		SaveFile(JpgData, Filename);
		return true;
	}

	bool SaveBmpFile(const TArray<FColor>& ImageData, int Width, int Height, const FString& Filename)
	{
		TArray<uint8> BmpData;
		ConvertToBmp(ImageData, Width, Height, BmpData);
		SaveFile(BmpData, Filename);
		return true;
	}

private:
	TSharedPtr<IImageWrapper> PngImageWrapper;
	TSharedPtr<IImageWrapper> JpgImageWrapper;

};
