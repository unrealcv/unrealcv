// Weichao Qiu @ 2017
#include "ImageUtil.h"
#include "Runtime/Core/Public/Serialization/BufferArchive.h"
#include "Runtime/ImageWrapper/Public/BmpImageSupport.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"
#include "UnrealcvStats.h"
#include "UnrealcvLog.h"

DECLARE_CYCLE_STAT(TEXT("FImageUtil::ConvertToPng"), STAT_ConvertToPng, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("FColorToJpg"), STAT_FColorToJpg, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("FColorToBmp"), STAT_FColorToBmp, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("SerializeBmpData"), STAT_SerializeBmp, STATGROUP_UnrealCV);
// DECLARE_CYCLE_STAT(TEXT("SaveFile"), STAT_SaveFile, STATGROUP_UnrealCV);

bool FImageUtil::ConvertToPng(const TArray<FColor>& ImageData, int Width, int Height, TArray<uint8>& PngData)
{
	SCOPE_CYCLE_COUNTER(STAT_ConvertToPng);

	if (ImageData.Num() == 0 || ImageData.Num() != Width * Height)
	{
		return false;
	}

	PngImageWrapper->SetRaw(ImageData.GetData(), ImageData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);
	PngData = PngImageWrapper->GetCompressed();
	return true;
}

bool FImageUtil::ConvertToJpg(const TArray<FColor>& ImageData, int Width, int Height, TArray<uint8>& JpgData)
{
	SCOPE_CYCLE_COUNTER(STAT_FColorToJpg);

	if (ImageData.Num() == 0 || ImageData.Num() != Width * Height)
	{
		return false;
	}

	JpgImageWrapper->SetRaw(ImageData.GetData(), ImageData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);
	JpgData = JpgImageWrapper->GetCompressed();
	return true;
}

/*
FArchive& operator<<(FArchive& Ar, FBitmapFileHeader& FileHeader)
{
	Ar << FileHeader.bfOffBits << FileHeader.bfReserved1 << FileHeader.bfReserved2 << FileHeader.bfSize << FileHeader.bfType;
	return Ar;
}
*/

bool FImageUtil::ConvertToBmp(const TArray<FColor>& ImageData, int Width, int Height, TArray<uint8>& BmpData)
{
	SCOPE_CYCLE_COUNTER(STAT_FColorToBmp);

	if (ImageData.Num() == 0 || ImageData.Num() != Width * Height)
	{
		return false;
	}

	FBitmapFileHeader BitmapFileHeader;
	// The header field used to identify the BMP and DIB file is 0x42 0x4D in hexadecimal, same as BM in ASCII. The following entries are possible:
	BitmapFileHeader.bfType = 0x4D42; // Check this, big endian or little endian?
	BitmapFileHeader.bfSize = sizeof(FBitmapFileHeader) + sizeof(FBitmapInfoHeader) + Width * Height * 4;
	BitmapFileHeader.bfOffBits = sizeof(FBitmapFileHeader) + sizeof(FBitmapInfoHeader);

	// Check, https://en.wikipedia.org/wiki/BMP_file_format
	FBitmapInfoHeader BitmapInfoHeader;
	BitmapInfoHeader.biSize = sizeof(FBitmapInfoHeader);
	check(BitmapInfoHeader.biSize == 40);
	BitmapInfoHeader.biWidth = Width;
	BitmapInfoHeader.biHeight = -Height; // Use a negative value to make the image up/side down
	// https://stackoverflow.com/questions/4669041/bmp-image-generated-but-displayed-inverted
	BitmapInfoHeader.biPlanes = 1;
	BitmapInfoHeader.biBitCount = 32;
	BitmapInfoHeader.biCompression = 0; // No compression
	BitmapInfoHeader.biSizeImage = 0; // Can be 0
	BitmapInfoHeader.biXPelsPerMeter = 1024; // Is this right?
	BitmapInfoHeader.biYPelsPerMeter = 1024;
	BitmapInfoHeader.biClrUsed = 0; // No color plate
	BitmapInfoHeader.biClrImportant = 0; // No color plate

	FBufferArchive Writer;
	Writer << BitmapFileHeader << BitmapInfoHeader;

	TArray<uint8> Bytes;
	Bytes.AddUninitialized(Width * Height * 4);
	{
		SCOPE_CYCLE_COUNTER(STAT_SerializeBmp);
		// Writer << ImageData; // Slow
		// ImageData.BulkSerialize(Writer); // Slow
		FMemory::Memcpy(Bytes.GetData(), ImageData.GetData(), Bytes.Num());
		Writer << Bytes;
	}
	BmpData = Writer;
	// Writer << BitmapInfoHeader;
	// Writer << ImageData;

	// JpgImageWrapper->SetRaw(ImageData.GetData(), ImageData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);
	// JpgData = JpgImageWrapper->GetCompressed(ImageCompression::Uncompressed);
	return true;
}



bool FImageUtil::SaveFile(const TArray<uint8>& BinaryData, const FString& Filename)
{
	// SCOPE_CYCLE_COUNTER(STAT_SaveFile);

	if (FFileHelper::SaveArrayToFile(BinaryData, *Filename))
	{
		return true;
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Can not save to file %s"), *Filename);
		return false;
	}
}
