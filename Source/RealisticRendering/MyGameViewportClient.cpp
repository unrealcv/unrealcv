// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "ImageUtils.h"
#include "MyGameViewportClient.h"

UMyGameViewportClient::UMyGameViewportClient()
{
	CaptureFinished.Get()->Create();
}

// Reimplement a GameViewportClient is required according to the discussion from here
// https://forums.unrealengine.com/showthread.php?50857-FViewPort-ReadPixels-crash-while-play-on-quot-standalone-Game-quot-mode
void UMyGameViewportClient::Draw(FViewport * Viewport, FCanvas * SceneCanvas)
{
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//this line is reaalllly important
	Super::Draw(Viewport, SceneCanvas);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//UE_LOG(Victory,Warning,TEXT("VICTORY GAME VIEWPORT Ticking!"));
	if (IsPendingSaveRequest)
	{
		// Save to Disk
		TArray<FColor> Bitmap;
		bool bScreenshotSuccessful = false;
		// UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
		// FViewport* InViewport = ViewportClient->Viewport;
		FViewport* InViewport = this->Viewport;
		// ViewportClient->GetEngineShowFlags()->SetMotionBlur(false);
		GetEngineShowFlags()->SetMotionBlur(false);
		bScreenshotSuccessful = GetViewportScreenShot(InViewport, Bitmap);
		// ViewportClient->GetHighResScreenshotConfig().MergeMaskIntoAlpha(Bitmap);
		// Ensure that all pixels' alpha is set to 255
		for (auto& Color : Bitmap)
		{
			Color.A = 255;
		}

		if (bScreenshotSuccessful)
		{
			FIntVector Size(InViewport->GetSizeXY().X, InViewport->GetSizeXY().Y, 0);
			// TODO: Need to blend alpha, a bit weird from screen.

			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(Size.X, Size.Y, Bitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *CaptureFilename);
			// return FExecStatus(Filename);
		}
		else
		{
			// return FExecStatus::Error("Fail to capture image"); // TODO: Handle error
		}
		IsPendingSaveRequest = false;
		
		// Signal an event the capture finished
		CaptureFinished.Get()->Trigger();
	}
}

void UMyGameViewportClient::CaptureScreen(const FString& Filename)
{
	CaptureFilename = Filename;
	IsPendingSaveRequest = true;
}