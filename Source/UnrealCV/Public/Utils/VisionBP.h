#pragma once

#include "VisionBP.generated.h"

/** A static bp library for computer vision */
UCLASS()
class UNREALCV_API UVisionBP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	static FString SerializeBoneInfo(USkeletalMeshComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	static bool SaveData(const FString& Data, const FString& Filename);

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	static bool AppendData(const FString& Data, const FString& Filename);

	/** FilenameFmt is the format string which contains %d */
	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	static FString FormatFrameFilename(FString FilenameFmt);

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	static bool SendMessageBP(const FString& Message);
};
