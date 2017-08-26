// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"
#include "UnrealCV.h"
#include "Float16Color.h"
#include "CvCameraComponent.generated.h"

/**
 *
 */
UCLASS(Blueprintable, ClassGroup = Camera, meta = (BlueprintSpawnableComponent))
class UCvCameraComponent : public UCameraComponent
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* DepthCaptureComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USceneCaptureComponent2D* NormalCaptureComponent;

	// Other choices
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) // Enable the user to edit the RenderTarget
	USceneCaptureComponent2D* LitCaptureComponent;
public:

	UCvCameraComponent();
	~UCvCameraComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ImageWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int ImageHeight;

	// UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	void CaptureDepth(TArray<FFloat16Color>& FloatImage, int& Width, int& Height);

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	void CaptureDepthToFile(FString Filename);

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	void CaptureNormal();

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	void CaptureImage(TArray<FColor>& Image, int& Width, int& Height);

	UFUNCTION(BlueprintCallable, Category = "UnrealCV")
	void CaptureImageToFile(FString Filename); // BP does not allow overload

	virtual void BeginPlay() override;
};
