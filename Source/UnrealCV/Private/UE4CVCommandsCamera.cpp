// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "UE4CVCommands.h"
#include "ViewMode.h"
#include "ImageUtils.h"

void UE4CVCommands::RegisterCommandsCamera()
{
	FDispatcherDelegate Cmd;
	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraLocation);
	CommandDispatcher->BindCommand("vget /camera/[uint]/location", Cmd, "Get camera location");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraRotation);
	CommandDispatcher->BindCommand("vget /camera/[uint]/rotation", Cmd, "Get camera rotation");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::SetCameraLocation);
	CommandDispatcher->BindCommand("vset /camera/[uint]/location [float] [float] [float]", Cmd, "Set camera location");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::SetCameraRotation);
	CommandDispatcher->BindCommand("vset /camera/[uint]/rotation [float] [float] [float]", Cmd, "Set camera rotation");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraView);
	CommandDispatcher->BindCommand("vget /camera/[uint]/view", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraViewMode);
	CommandDispatcher->BindCommand("vget /camera/[uint]/[str]", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	/*
	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraImage);
	CommandDispatcher->BindCommand("vget /camera/[uint]/image", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraDepth);
	CommandDispatcher->BindCommand("vget /camera/[uint]/depth", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraObjectMask);
	CommandDispatcher->BindCommand("vget /camera/[uint]/object_mask", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraNormal);
	CommandDispatcher->BindCommand("vget /camera/[uint]/normal", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraBaseColor);
	CommandDispatcher->BindCommand("vget /camera/[uint]/base_color", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename
	*/
}

FExecStatus UE4CVCommands::SetCameraLocation(const TArray<FString>& Args)
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

FExecStatus UE4CVCommands::SetCameraRotation(const TArray<FString>& Args)
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

FExecStatus UE4CVCommands::GetCameraRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		// FRotator CameraRotation = GetActorRotation();  // We need the rotation of the controller
		FRotator CameraRotation = Character->GetControlRotation();
		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);

		return FExecStatus::OK(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}

/*
FExecStatus UE4CVCommands::GetCameraDepth(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FExecStatus ExecStatus = CommandDispatcher->Exec("vset /mode/depth"); // TODO: Overwrite the + operator of FExecStatus
		return GetCameraView(Args);
	}
	return FExecStatus::InvalidArgument;
}
*/

/*
FExecStatus UE4CVCommands::GetCameraBaseColor(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FExecStatus ExecStatus = CommandDispatcher->Exec("vset /mode/normal"); // TODO: Overwrite the + operator of FExecStatus
		return GetCameraView(Args);
	}
	return FExecStatus::InvalidArgument;
}
*/

/*
FExecStatus UE4CVCommands::GetCameraNormal(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FExecStatus ExecStatus = CommandDispatcher->Exec("vset /mode/normal"); // TODO: Overwrite the + operator of FExecStatus
		return GetCameraView(Args);
	}
	return FExecStatus::InvalidArgument;
}
*/

/*
FExecStatus UE4CVCommands::GetCameraObjectMask(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FExecStatus ExecStatus = CommandDispatcher->Exec("vset /mode/object"); // TODO: Overwrite the + operator of FExecStatus
		return GetCameraView(Args);
	}
	return FExecStatus::InvalidArgument;
}
*/

/*
FExecStatus UE4CVCommands::GetCameraImage(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FExecStatus ExecStatus = CommandDispatcher->Exec("vset /mode/lit");
		return GetCameraView(Args);
	}
	return FExecStatus::InvalidArgument;
}
*/

FExecStatus UE4CVCommands::GetCameraLocation(const TArray<FString>& Args)
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

FExecStatus UE4CVCommands::GetCameraViewSync(const FString& FullFilename)
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

FExecStatus UE4CVCommands::GetCameraViewAsyncQuery(const FString& FullFilename)
{
		// Method 1: Use custom ViewportClient
		// UMyGameViewportClient* ViewportClient = (UMyGameViewportClient*)Character->GetWorld()->GetGameViewport();
		// ViewportClient->CaptureScreen(FullFilename);

		// Method2: System screenshot function
		FScreenshotRequest::RequestScreenshot(FullFilename, false, false); // This is an async operation

		// Implement 2, Start async and query
		// FPromiseDelegate PromiseDelegate = FPromiseDelegate::CreateRaw(this, &UE4CVCommands::CheckStatusScreenshot);
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
FExecStatus UE4CVCommands::GetCameraViewAsyncCallback(const FString& FullFilename)
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

FExecStatus UE4CVCommands::GetCameraViewMode(const TArray<FString>& Args)
{
	if (Args.Num() == 2) // The first is camera id, the second is ViewMode
	{ 
		FString CameraId = Args[0];
		FString ViewMode = Args[1];

		/*
		TArray<FString> Args1;
		Args1.Add(ViewMode);
		FViewMode::Get().SetMode(Args1);
		*/
		FExecStatus ExecStatus = CommandDispatcher->Exec(FString::Printf(TEXT("vset /mode/%s"), *ViewMode));
		if (ExecStatus != FExecStatusType::OK)
		{
			return ExecStatus;
		}

		TArray<FString> Args2;
		Args2.Add(CameraId);
		ExecStatus = GetCameraView(Args2);
		return ExecStatus;
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::GetCameraView(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]);

		static uint32 NumCaptured = 0;
		NumCaptured++;
		FString Filename = FString::Printf(TEXT("%04d.png"), NumCaptured);
		const FString Dir = FPlatformProcess::BaseDir(); // TODO: Change this to screen capture folder
		FString FullFilename = FPaths::Combine(*Dir, *Filename);
		
		return this->GetCameraViewAsyncQuery(FullFilename);
	}
	return FExecStatus::InvalidArgument;
}

