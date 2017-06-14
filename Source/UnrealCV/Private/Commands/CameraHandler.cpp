// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "CameraHandler.h"
#include "ViewMode.h"
#include "ImageUtils.h"
#include "ImageWrapper.h"
#include "GTCaptureComponent.h"
#include "PlayerViewMode.h"
#include "UE4CVServer.h"
#include "CaptureManager.h"
#include "CineCameraActor.h"
#include "ObjectPainter.h"
#include "ScreenCapture.h"

FString GetDiskFilename(FString Filename)
{
	const FString Dir = FPlatformProcess::BaseDir(); // TODO: Change this to screen capture folder
	// const FString Dir = FPaths::ScreenShotDir();
	FString FullFilename = FPaths::Combine(*Dir, *Filename);

	FString DiskFilename = IFileManager::Get().GetFilenameOnDisk(*FullFilename); // This is important
	return DiskFilename;
}

FString GenerateSeqFilename()
{
	static uint32 NumCaptured = 0;
	NumCaptured++;
	FString Filename = FString::Printf(TEXT("%08d.png"), NumCaptured);
	return Filename;
}

void FCameraCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraViewMode);
	CommandDispatcher->BindCommand("vget /camera/[uint]/[str]", Cmd, "Get snapshot from camera, the third parameter is optional");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraViewMode);
	CommandDispatcher->BindCommand("vget /camera/[uint]/[str] [str]", Cmd, "Get snapshot from camera, the third parameter is optional");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetLitViewMode);
	CommandDispatcher->BindCommand("vget /camera/[uint]/lit", Cmd, "Get snapshot from camera, the third parameter is optional");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetLitViewMode);
	CommandDispatcher->BindCommand("vget /camera/[uint]/lit [str]", Cmd, "Get snapshot from camera, the third parameter is optional");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetObjectInstanceMask);
	CommandDispatcher->BindCommand("vget /camera/[uint]/object_mask", Cmd, "Get snapshot from camera, the third parameter is optional");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetObjectInstanceMask);
	Help = "Get object mask from camera";
	CommandDispatcher->BindCommand("vget /camera/[uint]/object_mask [str]", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetScreenshot);
	Help = "Get snapshot from camera";
	CommandDispatcher->BindCommand("vget /camera/[uint]/screenshot", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraLocation);
	Help = "Get camera location [x, y, z]";
	CommandDispatcher->BindCommand("vget /camera/[uint]/location", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraRotation);
	Help = "Get camera rotation [pitch, yaw, roll]";
	CommandDispatcher->BindCommand("vget /camera/[uint]/rotation", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::SetCameraLocation);
	Help = "Teleport camera to location [x, y, z]";
	CommandDispatcher->BindCommand("vset /camera/[uint]/location [float] [float] [float]", Cmd, Help);

	/** This is different from SetLocation (which is teleport) */
	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::MoveTo);
	Help = "Move camera to location [x, y, z], will be blocked by objects";
	CommandDispatcher->BindCommand("vset /camera/[uint]/moveto [float] [float] [float]", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::SetCameraRotation);
	Help = "Set rotation [pitch, yaw, roll] of camera [id]";
	CommandDispatcher->BindCommand("vset /camera/[uint]/rotation [float] [float] [float]", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraProjMatrix);
	Help = "Get projection matrix from camera [id]";
	CommandDispatcher->BindCommand("vget /camera/[uint]/proj_matrix", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(&FPlayerViewMode::Get(), &FPlayerViewMode::SetMode);
	Help = "Set ViewMode to (lit, normal, depth, object_mask)";
	// CommandDispatcher->BindCommand("vset /viewmode lit", Cmd, Help); // Better to check the correctness at compile time
	// CommandDispatcher->BindCommand("vset /viewmode normal", Cmd, Help); // Better to check the correctness at compile time
	// CommandDispatcher->BindCommand("vset /viewmode depth", Cmd, Help); // Better to check the correctness at compile time
	// CommandDispatcher->BindCommand("vset /viewmode object_mask", Cmd, Help); // Better to check the correctness at compile time
	CommandDispatcher->BindCommand("vset /viewmode [str]", Cmd, Help); // Better to check the correctness at compile time

	Cmd = FDispatcherDelegate::CreateRaw(&FPlayerViewMode::Get(), &FPlayerViewMode::GetMode);
	Help = "Get current ViewMode";
	CommandDispatcher->BindCommand("vget /viewmode", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetActorLocation);
	Help = "Get actor location [x, y, z]";
	CommandDispatcher->BindCommand("vget /actor/location", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetActorRotation);
	Help = "Get actor rotation [pitch, yaw, roll]";
	CommandDispatcher->BindCommand("vget /actor/rotation", Cmd, Help);

	// Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetBuffer);
	// CommandDispatcher->BindCommand("vget /camera/[uint]/buffer", Cmd, "Get buffer of this camera");

	Help = "Return raw binary image data, instead of the image filename";
	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetPngBinary(Args, TEXT("lit")); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/lit png", Cmd, Help);
	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetPngBinary(Args, TEXT("depth")); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/depth png", Cmd, Help);
	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetPngBinary(Args, TEXT("normal")); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/normal png", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetNpyBinary(Args, TEXT("depth")); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/depth npy", Cmd, Help);

	// TODO: object_mask will be handled differently
}

FExecStatus FCameraCommandHandler::GetCameraProjMatrix(const TArray<FString>& Args)
{
	// FMatrix& ProjMatrix = FSceneView::ViewProjectionMatrix;
	// this->Character->GetWorld()->GetGameViewport()->Viewport->
	// this->Character
	return FExecStatus::InvalidArgument;
}

FExecStatus FCameraCommandHandler::MoveTo(const TArray<FString>& Args)
{
	/** The API for Character, Pawn and Actor are different */
	if (Args.Num() == 4) // ID, X, Y, Z
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
		FVector Location = FVector(X, Y, Z);

		bool Sweep = true;
		// if sweep is true, the object can not move through another object
		// Check invalid location and move back a bit.
		bool Success = FUE4CVServer::Get().GetPawn()->SetActorLocation(Location, Sweep, NULL, ETeleportType::TeleportPhysics);

		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FCameraCommandHandler::SetCameraLocation(const TArray<FString>& Args)
{
	/** The API for Character, Pawn and Actor are different */
	if (Args.Num() == 4) // ID, X, Y, Z
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
		FVector Location = FVector(X, Y, Z);

		bool Sweep = false;
		// if sweep is true, the object can not move through another object
		// Check invalid location and move back a bit.

		bool Success = FUE4CVServer::Get().GetPawn()->SetActorLocation(Location, Sweep, NULL, ETeleportType::TeleportPhysics);

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
		APawn* Pawn = FUE4CVServer::Get().GetPawn();
		AController* Controller = Pawn->GetController();
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
		bool bIsMatinee = false;

		FRotator CameraRotation;
		ACineCameraActor* CineCameraActor = nullptr;
		for (AActor* Actor : this->GetWorld()->GetCurrentLevel()->Actors)
		{
			// if (Actor && Actor->IsA(AMatineeActor::StaticClass())) // AMatineeActor is deprecated
			if (Actor && Actor->IsA(ACineCameraActor::StaticClass()))
			{
				bIsMatinee = true;
				CameraRotation = Actor->GetActorRotation();
				break;
			}
		}

		if (!bIsMatinee)
		{
			int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
			APawn* Pawn = FUE4CVServer::Get().GetPawn();
			CameraRotation = Pawn->GetControlRotation();
		}

		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);

		return FExecStatus::OK(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}

FExecStatus FCameraCommandHandler::GetCameraLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		bool bIsMatinee = false;

		FVector CameraLocation;
		ACineCameraActor* CineCameraActor = nullptr;
		for (AActor* Actor : this->GetWorld()->GetCurrentLevel()->Actors)
		{
			// if (Actor && Actor->IsA(AMatineeActor::StaticClass())) // AMatineeActor is deprecated
			if (Actor && Actor->IsA(ACineCameraActor::StaticClass()))
			{
				bIsMatinee = true;
				CameraLocation = Actor->GetActorLocation();
				break;
			}
		}

		if (!bIsMatinee)
		{
			int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
			// APawn* Pawn = FUE4CVServer::Get().GetPawn();
			// CameraLocation = Pawn->GetActorLocation();
			UGTCaptureComponent* CaptureComponent = FCaptureManager::Get().GetCamera(CameraId);
			if (CaptureComponent == nullptr)
			{
				return FExecStatus::Error(FString::Printf(TEXT("Camera %d can not be found."), CameraId));
			}
			CameraLocation = CaptureComponent->GetComponentLocation();
		}

		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraLocation.X, CameraLocation.Y, CameraLocation.Z);

		return FExecStatus::OK(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}

FExecStatus FCameraCommandHandler::GetObjectInstanceMask(const TArray<FString>& Args)
{
	if (Args.Num() <= 2) // The first is camera id, the second is ViewMode
	{
		// Use command dispatcher is more universal
		FExecStatus ExecStatus = CommandDispatcher->Exec(TEXT("vset /viewmode object_mask"));
		if (ExecStatus != FExecStatusType::OK)
		{
			return ExecStatus;
		}

		ExecStatus = GetScreenshot(Args);
		return ExecStatus;
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FCameraCommandHandler::GetLitViewMode(const TArray<FString>& Args)
{
	if (Args.Num() <= 3)
	{
		// For this viewmode, The post-effect material needs to be explictly cleared
		FPlayerViewMode::Get().Lit();

		TArray<FString> ExtraArgs(Args);
		ExtraArgs.Insert(TEXT("lit"), 1);
		return GetCameraViewMode(ExtraArgs);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FCameraCommandHandler::GetCameraViewMode(const TArray<FString>& Args)
{
	if (Args.Num() <= 3) // The first is camera id, the second is ViewMode
	{
		int32 CameraId = FCString::Atoi(*Args[0]);
		FString ViewMode = Args[1];

		FString Filename;
		if (Args.Num() == 3)
		{
			Filename = Args[2];
		}
		else
		{
			Filename = GenerateSeqFilename();
		}

		UGTCaptureComponent* GTCapturer = FCaptureManager::Get().GetCamera(CameraId);
		if (GTCapturer == nullptr)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Invalid camera id %d"), CameraId));
		}

		FAsyncRecord* AsyncRecord = GTCapturer->Capture(*ViewMode, *Filename); // Due to sandbox implementation of UE4, it is not possible to specify an absolute path directly.
		if (AsyncRecord == nullptr)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Unrecognized capture mode %s"), *ViewMode));
		}

		// TODO: Check IsPending is problematic.
		FPromiseDelegate PromiseDelegate = FPromiseDelegate::CreateLambda([Filename, AsyncRecord]()
		{
			if (AsyncRecord->bIsCompleted)
			{
				AsyncRecord->Destory();
				return FExecStatus::OK(GetDiskFilename(Filename));
			}
			else
			{
				return FExecStatus::Pending();
			}
		});
		FString Message = FString::Printf(TEXT("File will be saved to %s"), *Filename);
		return FExecStatus::AsyncQuery(FPromise(PromiseDelegate));
		// The filename here is just for message, not the fullname on the disk, because we can not know that due to sandbox issue.
	}
	return FExecStatus::InvalidArgument;
}

/** vget /camera/[id]/screenshot */
FExecStatus FCameraCommandHandler::GetScreenshot(const TArray<FString>& Args)
{
	int32 CameraId = FCString::Atoi(*Args[0]);

	FString Filename;
	if (Args.Num() > 2)
	{
		return FExecStatus::InvalidArgument;
	}
	if (Args.Num() == 1)
	{
		Filename = GenerateSeqFilename();
	}
	if (Args.Num() == 2)
	{
		Filename = Args[1];
	}

	if (Filename.ToLower() == TEXT("png"))
	{
		return ScreenCaptureAsyncByQuery(); // return the binary data
	}
	else
	{
		return ScreenCaptureAsyncByQuery(Filename);
	}
}

FExecStatus FCameraCommandHandler::GetBuffer(const TArray<FString>& Args)
{
	// Initialize console variables.
	static IConsoleVariable* ICVar1 = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationDumpFrames"));
	// r.BufferVisualizationDumpFramesAsHDR
	ICVar1->Set(1, ECVF_SetByCode);
	static IConsoleVariable* ICVar2 = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BufferVisualizationDumpFramesAsHDR"));
	ICVar2->Set(1, ECVF_SetByCode);

	/*
	FHighResScreenshotConfig Config = GetHighResScreenshotConfig();
	Config.bCaptureHDR = true;
	Config.bDumpBufferVisualizationTargets = true;
	*/

	GIsHighResScreenshot = true;

	// Get the viewport
	// FSceneViewport* SceneViewport = this->GetWorld()->GetGameViewport()->GetGameViewport();
	// SceneViewport->TakeHighResScreenShot();
	return FExecStatus::OK();
}

FExecStatus FCameraCommandHandler::GetActorRotation(const TArray<FString>& Args)
{
	APawn* Pawn = FUE4CVServer::Get().GetPawn();
	FRotator CameraRotation = Pawn->GetControlRotation();
	FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);
	return FExecStatus::OK(Message);
}

FExecStatus FCameraCommandHandler::GetActorLocation(const TArray<FString>& Args)
{
	APawn* Pawn = FUE4CVServer::Get().GetPawn();
	FVector CameraLocation = Pawn->GetActorLocation();
	FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraLocation.X, CameraLocation.Y, CameraLocation.Z);
	return FExecStatus::OK(Message);
}

FExecStatus FCameraCommandHandler::GetPngBinary(const TArray<FString>& Args, const FString& ViewMode)
{
	int32 CameraId = FCString::Atoi(*Args[0]);

	UGTCaptureComponent* GTCapturer = FCaptureManager::Get().GetCamera(CameraId);
	if (GTCapturer == nullptr)
	{
		return FExecStatus::Error(FString::Printf(TEXT("Invalid camera id %d"), CameraId));
	}

	TArray<uint8> ImgData = GTCapturer->CapturePng(ViewMode);
	return FExecStatus::Binary(ImgData);
}

FExecStatus FCameraCommandHandler::GetNpyBinary(const TArray<FString>& Args, const FString& ViewMode)
{
	int32 CameraId = FCString::Atoi(*Args[0]);

	UGTCaptureComponent* GTCapturer = FCaptureManager::Get().GetCamera(CameraId);
	if (GTCapturer == nullptr)
	{
		return FExecStatus::Error(FString::Printf(TEXT("Invalid camera id %d"), CameraId));
	}

	TArray<uint8> ImgData = GTCapturer->CaptureNpy(ViewMode);
	return FExecStatus::Binary(ImgData);
}
