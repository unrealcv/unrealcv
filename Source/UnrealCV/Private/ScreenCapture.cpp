#include "UnrealCVPrivate.h"
#include "Commands/CameraHandler.h"

/** Deprecated: Get camera view in a sync way, can not work in standalone mode */
// FExecStatus GetCameraViewSync(const FString& Fullfilename);
/** Get camera view in async way, the return FExecStaus is in pending status and need to check the promise to get result */
// FExecStatus GetCameraViewAsyncQuery(const FString& Fullfilename);
// FExecStatus GetCameraViewAsyncCallback(const FString& Fullfilename);

// Sync operation for screen capture
bool DoCaptureScreen(UGameViewportClient *ViewportClient, const FString& CaptureFilename)
{
		bool bScreenshotSuccessful = false;
		FViewport* InViewport = ViewportClient->Viewport;
		ViewportClient->GetEngineShowFlags()->SetMotionBlur(false);
		FIntVector Size(InViewport->GetSizeXY().X, InViewport->GetSizeXY().Y, 0);

		bool IsHDR = true;

		if (!IsHDR)
		{
			TArray<FColor> Bitmap;
			bScreenshotSuccessful = GetViewportScreenShot(InViewport, Bitmap);
			// InViewport->ReadFloat16Pixels

			if (bScreenshotSuccessful)
			{
				// Ensure that all pixels' alpha is set to 255
				for (auto& Color : Bitmap)
				{
					Color.A = 255;
				}
				// TODO: Need to blend alpha, a bit weird from screen.

				TArray<uint8> CompressedBitmap;
				FImageUtils::CompressImageArray(Size.X, Size.Y, Bitmap, CompressedBitmap);
				FFileHelper::SaveArrayToFile(CompressedBitmap, *CaptureFilename);
			}
		}
		else // Capture HDR, unable to read float16 data from here. Need to use a rendertarget.
		{
			CaptureFilename.Replace(TEXT(".png"), TEXT(".exr"));
			TArray<FFloat16Color> FloatBitmap;
			FloatBitmap.AddZeroed(Size.X * Size.Y);
			InViewport->ReadFloat16Pixels(FloatBitmap);

			IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
			IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);

			ImageWrapper->SetRaw(FloatBitmap.GetData(), FloatBitmap.GetAllocatedSize(), Size.X, Size.Y, ERGBFormat::RGBA, 16);
			const TArray<uint8>& PngData = ImageWrapper->GetCompressed(ImageCompression::Uncompressed);
			FFileHelper::SaveArrayToFile(PngData, *CaptureFilename);
		}

		return bScreenshotSuccessful;
}

FExecStatus FScreenCapture::GetCameraViewAsyncQuery(const FString& FullFilename)
{
		// Method 1: Use custom ViewportClient
		// UMyGameViewportClient* ViewportClient = (UMyGameViewportClient*)Character->GetWorld()->GetGameViewport();
		// ViewportClient->CaptureScreen(FullFilename);

		// Method2: System screenshot function
		FScreenshotRequest::RequestScreenshot(FullFilename, false, false); // This is an async operation

		// Implement 2, Start async and query
		// FPromiseDelegate PromiseDelegate = FPromiseDelegate::CreateRaw(this, &FCameraCommandHandler::CheckStatusScreenshot);
		FPromiseDelegate PromiseDelegate = FPromiseDelegate::CreateLambda([FullFilename]()
		{
			if (FScreenshotRequest::IsScreenshotRequested())
			{
				return FExecStatus::Pending();
			}
			else
			{
				FString DiskFilename = IFileManager::Get().GetFilenameOnDisk(*FullFilename); // This is important
				// See: https://wiki.unrealengine.com/Packaged_Game_Paths,_Obtain_Directories_Based_on_Executable_Location.
				return FExecStatus::OK(DiskFilename);
			}
		});

		// Method3: USceneCaptureComponent2D, inspired by StereoPanorama plugin

		FExecStatus ExecStatusQuery = FExecStatus::AsyncQuery(FPromise(PromiseDelegate));
		/* This is only valid within custom viewport client
		UGameViewportClient* ViewportClient = Character->GetWorld()->GetGameViewport();
		ViewportClient->OnScreenshotCaptured().Clear(); // This is required to handle the filename issue.
		ViewportClient->OnScreenshotCaptured().AddLambda(
			[FullFilename](int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
		{
			// Save bitmap to disk
			TArray<FColor>& RefBitmap = const_cast<TArray<FColor>&>(Bitmap);
			for (auto& Color : RefBitmap)
			{
				Color.A = 255;
			}

			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(SizeX, SizeY, RefBitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *FullFilename);
		});
		*/
		return ExecStatusQuery;

}

/*
FExecStatus FCameraCommandHandler::GetCameraViewAsyncCallback(const FString& FullFilename)
{
		FScreenshotRequest::RequestScreenshot(FullFilename, false, false); // This is an async operation

		// async implement 1, Start async and callback
		FExecStatus ExecStatusAsyncCallback = FExecStatus::AsyncCallback();
		int32 TaskId = ExecStatusAsyncCallback.TaskId;
		UMyGameViewportClient* ViewportClient = (UMyGameViewportClient*)Character->GetWorld()->GetGameViewport();
		ViewportClient->OnScreenshotCaptured().Clear();
		ViewportClient->OnScreenshotCaptured().AddLambda(
			[FullFilename, TaskId](int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap)
		{
			// Save bitmap to disk
			TArray<FColor>& RefBitmap = const_cast<TArray<FColor>&>(Bitmap);
			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(SizeX, SizeY, RefBitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *FullFilename);

			FString Message = FullFilename;
			// FAsyncTaskPool::Get().CompleteTask(TaskId, Message); // int32 is easy to pass around in lamdba

			// Mark this async task finished
			// ExecStatusAsyncCallback.MessageBody = FullFilename;
			// ExecStatusAsyncCallback.OnFinished().Execute(); // FullFilename is the message to return
		});
		return ExecStatusAsyncCallback;
}
*/

FExecStatus GetCameraViewSync(const FString& FullFilename)
{
	// This can only work within editor
	// Reimplement a GameViewportClient is required according to the discussion from here
	// https://forums.unrealengine.com/showthread.php?50857-FViewPort-ReadPixels-crash-while-play-on-quot-standalone-Game-quot-mode
	UGameViewportClient* ViewportClient = GWorld->GetGameViewport();
	if (DoCaptureScreen(ViewportClient, FullFilename))
	{
		return FExecStatus::OK(FullFilename);
	}
	else
	{
		return FExecStatus::Error("Fail to capture screen");
	}
}
