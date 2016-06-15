// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "CommandHandler.h"
#include "ViewMode.h"
#include "ImageUtils.h"

void FCameraCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraLocation);
	CommandDispatcher->BindCommand("vget /camera/[uint]/location", Cmd, "Get camera location");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraRotation);
	CommandDispatcher->BindCommand("vget /camera/[uint]/rotation", Cmd, "Get camera rotation");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::SetCameraLocation);
	CommandDispatcher->BindCommand("vset /camera/[uint]/location [float] [float] [float]", Cmd, "Set camera location");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::SetCameraRotation);
	CommandDispatcher->BindCommand("vset /camera/[uint]/rotation [float] [float] [float]", Cmd, "Set camera rotation");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraView);
	CommandDispatcher->BindCommand("vget /camera/[uint]/view", Cmd, "Get snapshot from camera, the second parameter is optional"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraView);
	CommandDispatcher->BindCommand("vget /camera/[uint]/view [str]", Cmd, "Get snapshot from camera, the second parameter is optional"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraViewMode);
	CommandDispatcher->BindCommand("vget /camera/[uint]/[str]", Cmd, "Get snapshot from camera, the third parameter is optional"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraViewMode);
	CommandDispatcher->BindCommand("vget /camera/[uint]/[str] [str]", Cmd, "Get snapshot from camera, the third parameter is optional"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(&FViewMode::Get(), &FViewMode::SetMode);
	CommandDispatcher->BindCommand("vset /mode [str]", Cmd, "Set mode"); // Better to check the correctness at compile time

	Cmd = FDispatcherDelegate::CreateRaw(&FViewMode::Get(), &FViewMode::GetMode);
	CommandDispatcher->BindCommand("vget /mode", Cmd, "Get mode");
}

FExecStatus FCameraCommandHandler::SetCameraLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 4) // ID, X, Y, Z
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
		FVector Location = FVector(X, Y, Z);
		Character->SetActorLocation(Location);

		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FCameraCommandHandler::SetCameraRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 4) // ID, Pitch, Roll, Yaw
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
		FRotator Rotator = FRotator(Pitch, Yaw, Roll);
		AController* Controller = Character->GetController();
		Controller->ClientSetRotation(Rotator); // Teleport action
		// SetActorRotation(Rotator);  // This is not working

		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FCameraCommandHandler::GetCameraRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		// FRotator CameraRotation = this->Character->GetActorRotation();  // We need the rotation of the controller
		FRotator CameraRotation = Character->GetControlRotation();
		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);

		return FExecStatus::OK(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}

FExecStatus FCameraCommandHandler::GetCameraLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		FVector CameraLocation = Character->GetActorLocation();
		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraLocation.X, CameraLocation.Y, CameraLocation.Z);

		return FExecStatus::OK(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}

// Sync operation for screen capture
bool DoCaptureScreen(UGameViewportClient *ViewportClient, const FString& CaptureFilename)
{
		TArray<FColor> Bitmap;
		bool bScreenshotSuccessful = false;
		FViewport* InViewport = ViewportClient->Viewport;
		ViewportClient->GetEngineShowFlags()->SetMotionBlur(false);
		bScreenshotSuccessful = GetViewportScreenShot(InViewport, Bitmap);

		if (bScreenshotSuccessful)
		{
			// Ensure that all pixels' alpha is set to 255
			for (auto& Color : Bitmap)
			{
				Color.A = 255;
			}
			FIntVector Size(InViewport->GetSizeXY().X, InViewport->GetSizeXY().Y, 0);
			// TODO: Need to blend alpha, a bit weird from screen.

			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(Size.X, Size.Y, Bitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *CaptureFilename);
		}
		return bScreenshotSuccessful;
}

FExecStatus FCameraCommandHandler::GetCameraViewSync(const FString& FullFilename)
{
	// This can only work within editor
	// Reimplement a GameViewportClient is required according to the discussion from here
	// https://forums.unrealengine.com/showthread.php?50857-FViewPort-ReadPixels-crash-while-play-on-quot-standalone-Game-quot-mode
	UGameViewportClient* ViewportClient = Character->GetWorld()->GetGameViewport();
	if (DoCaptureScreen(ViewportClient, FullFilename))
	{
		return FExecStatus::OK(FullFilename);
	}
	else
	{
		return FExecStatus::Error("Fail to capture screen");
	}
}

FExecStatus FCameraCommandHandler::GetCameraViewAsyncQuery(const FString& FullFilename)
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
		FExecStatus ExecStatusQuery = FExecStatus::AsyncQuery(FPromise(PromiseDelegate));
		/*
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

FExecStatus FCameraCommandHandler::GetCameraViewMode(const TArray<FString>& Args)
{
	if (Args.Num() <= 3) // The first is camera id, the second is ViewMode
	{
		FString CameraId = Args[0];
		FString ViewMode = Args[1];

		/*
		TArray<FString> Args1;
		Args1.Add(ViewMode);
		FViewMode::Get().SetMode(Args1);
		*/
		// Use command dispatcher is more universal
		FExecStatus ExecStatus = CommandDispatcher->Exec(FString::Printf(TEXT("vset /mode %s"), *ViewMode));
		if (ExecStatus != FExecStatusType::OK)
		{
			return ExecStatus;
		}

		TArray<FString> Args2;
		Args2.Add(CameraId);
		if (Args.Num() == 3)
		{
			FString Fullfilename = Args[2];
			Args2.Add(Fullfilename);
		}
		ExecStatus = GetCameraView(Args2);
		return ExecStatus;
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FCameraCommandHandler::GetCameraView(const TArray<FString>& Args)
{
	if (Args.Num() <= 2)
	{
		int32 CameraId = FCString::Atoi(*Args[0]);
		static uint32 NumCaptured = 0;

		FString FullFilename, Filename;
		if (Args.Num() == 1)
		{
			NumCaptured++;
			Filename = FString::Printf(TEXT("%04d.png"), NumCaptured);
		}
		if (Args.Num() == 2)
		{
			Filename = Args[1];
		}
		const FString Dir = FPlatformProcess::BaseDir(); // TODO: Change this to screen capture folder
		// const FString Dir = FPaths::ScreenShotDir();
		FullFilename = FPaths::Combine(*Dir, *Filename);

		return this->GetCameraViewAsyncQuery(FullFilename);
	}
	return FExecStatus::InvalidArgument;
}
