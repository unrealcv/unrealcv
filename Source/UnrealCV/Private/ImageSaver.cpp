
#include "UnrealCVPrivate.h"
#include "ImageWrapper.h"
#include "ImageSaver.h"
#include "UnrealCV.h"

bool FImageSaver::SaveFile(const TArray<FColor>& Image, int Width, int Height, FString Filename)
{
	if (Filename.EndsWith(".png", ESearchCase::IgnoreCase))
	{
		return FImageSaver::SavePng(Image, Width, Height, Filename);
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not recognize file format from filename %s"), *Filename);
		return false;
	}
}


bool FImageSaver::SaveFile(const TArray<FFloat16Color>& FloatImage, int Width, int Height, FString Filename)
{
	static IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	if (Filename.EndsWith(".exr", ESearchCase::IgnoreCase))
	{
		return FImageSaver::SaveExr(FloatImage, Width, Height, Filename);
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not recognize file format from filename %s"), *Filename);
		return false;
	}
}


bool FImageSaver::SavePng(const TArray<FColor>& Image, int Width, int Height, FString Filename)
{
	static IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	static IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	ImageWrapper->SetRaw(Image.GetData(), Image.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);
	const TArray<uint8>& ImageData = ImageWrapper->GetCompressed(ImageCompression::Uncompressed);
	FFileHelper::SaveArrayToFile(ImageData, *Filename);
	FString FullFilename = FPaths::ConvertRelativePathToFull(Filename); // This might not be the real path on the disk due to sandbox.
	UE_LOG(LogUnrealCV, Log, TEXT("Save to %s"), *FullFilename);
	return true;
}

bool FImageSaver::SaveExr(const TArray<FFloat16Color>& FloatImage, int Width, int Height, FString Filename)
{
	static IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	static IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);

	ImageWrapper->SetRaw(FloatImage.GetData(), FloatImage.GetAllocatedSize(), Width, Height, ERGBFormat::RGBA, 16);
	const TArray<uint8>& ImageData = ImageWrapper->GetCompressed(ImageCompression::Uncompressed);
	FFileHelper::SaveArrayToFile(ImageData, *Filename);
	FString FullFilename = FPaths::ConvertRelativePathToFull(Filename); // This might not be the real path on the disk due to sandbox.
	UE_LOG(LogUnrealCV, Log, TEXT("Save to %s"), *FullFilename);
	return true;
}
