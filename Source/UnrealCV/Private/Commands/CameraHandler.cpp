// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "CameraHandler.h"
#include "ViewMode.h"
#include "ImageUtils.h"
#include "IImageWrapper.h"
#include "GTCaptureComponent.h"
#include "PlayerViewMode.h"
#include "UE4CVServer.h"
#include "CaptureManager.h"
#include "CineCameraActor.h"
#include "ObjectPainter.h"
#include "ScreenCapture.h"
#include "Serialization.h"

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

  Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraPose);
  Help = "Get camera location [x, y, z] and rotation [pitch, yaw, roll]";
  CommandDispatcher->BindCommand("vget /camera/[uint]/pose", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::SetCameraLocation);
	Help = "Teleport camera to location [x, y, z]";
	CommandDispatcher->BindCommand("vset /camera/[uint]/location [float] [float] [float]", Cmd, Help);

  Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::SetCameraRotation);
  Help = "Set rotation [pitch, yaw, roll] of camera [id]";
  CommandDispatcher->BindCommand("vset /camera/[uint]/rotation [float] [float] [float]", Cmd, Help);

  Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::SetCameraPose);
  Help = "Teleport camera to location [x, y, z] and rotation [pitch, yaw, roll]";
  CommandDispatcher->BindCommand("vset /camera/[uint]/pose [float] [float] [float] [float] [float] [float]", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraHorizontalFieldOfView);
	Help = "Get camera horizontal field of view";
	CommandDispatcher->BindCommand("vget /camera/[uint]/horizontal_fieldofview", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::SetCameraHorizontalFieldOfView);
	Help = "Set camera horizontal field of view";
	CommandDispatcher->BindCommand("vset /camera/[uint]/horizontal_fieldofview [float]", Cmd, Help);

	/** This is different from SetLocation (which is teleport) */
	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::MoveTo);
	Help = "Move camera to location [x, y, z], will be blocked by objects";
	CommandDispatcher->BindCommand("vset /camera/[uint]/moveto [float] [float] [float]", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetCameraProjMatrix);
	Help = "Get projection matrix from camera [id]";
	CommandDispatcher->BindCommand("vget /camera/[uint]/proj_matrix", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(&FPlayerViewMode::Get(), &FPlayerViewMode::SetMode);
	Help = "Set ViewMode to (lit, normal, depth, object_mask)";
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

	Help = "Return raw binary image data, instead of the image filename";
	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return GetPngBinary(Args, TEXT("lit")); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/lit png", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return GetPngBinary(Args, TEXT("depth")); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/depth png", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return GetPngBinary(Args, TEXT("normal")); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/normal png", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetNpyBinaryUint8(Args, TEXT("lit"), 4); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/lit npy", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetNpyBinaryFloat16(Args, TEXT("depth"), 1); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/depth npy", Cmd, Help);

 	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetNpyBinaryFloat16(Args, TEXT("plane_depth"), 1); });
 	CommandDispatcher->BindCommand("vget /camera/[uint]/plane_depth npy", Cmd, Help);

 	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetNpyBinaryFloat16(Args, TEXT("vis_depth"), 1); });
 	CommandDispatcher->BindCommand("vget /camera/[uint]/vis_depth npy", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) { return this->GetNpyBinaryUint8(Args, TEXT("normal"), 3); });
	CommandDispatcher->BindCommand("vget /camera/[uint]/normal npy", Cmd, Help);

	Cmd = FDispatcherDelegate::CreateLambda([this](const TArray<FString>& Args) {
		TArray<uint8> LitData = this->GetNpyBinaryUint8Data(Args, TEXT("lit"), 4);
		TArray<uint8> DepthData = this->GetNpyBinaryFloat16Data(Args, TEXT("depth"), 1);
		TArray<uint8> NormalData = this->GetNpyBinaryUint8Data(Args, TEXT("normal"), 3);
		TArray<uint8> Data;
		Data += LitData;
		Data += DepthData;
		Data += NormalData;
		return FExecStatus::Binary(Data);
	});
	CommandDispatcher->BindCommand("vget /camera/[uint]/lit_depth_normal npy", Cmd, Help);

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

FExecStatus FCameraCommandHandler::SetCameraPose(const TArray<FString>& Args)
{
  if (Args.Num() == 7) // ID, X, Y, Z, Pitch, Roll, Yaw
  {
    int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
    float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
    FVector Location = FVector(X, Y, Z);
    float Pitch = FCString::Atof(*Args[4]), Yaw = FCString::Atof(*Args[5]), Roll = FCString::Atof(*Args[6]);
    FRotator Rotator = FRotator(Pitch, Yaw, Roll);

    APawn* Pawn = FUE4CVServer::Get().GetPawn();

    bool Sweep = false;
    bool Success = Pawn->SetActorLocation(Location, Sweep, NULL, ETeleportType::TeleportPhysics);

    AController* Controller = Pawn->GetController();
    Controller->ClientSetRotation(Rotator); // Teleport action
    // SetActorRotation(Rotator);  // This is not working

    return FExecStatus::OK();
  }
  return FExecStatus::InvalidArgument;
}

FExecStatus FCameraCommandHandler::GetCameraPose(const TArray<FString>& Args)
{
  if (Args.Num() == 1)
  {
    bool bIsMatinee = false;

    FVector CameraLocation;
    FRotator CameraRotation;
    ACineCameraActor* CineCameraActor = nullptr;
    for (AActor* Actor : this->GetWorld()->GetCurrentLevel()->Actors)
    {
      // if (Actor && Actor->IsA(AMatineeActor::StaticClass())) // AMatineeActor is deprecated
      if (Actor && Actor->IsA(ACineCameraActor::StaticClass()))
      {
        bIsMatinee = true;
        CameraLocation = Actor->GetActorLocation();
        CameraRotation = Actor->GetActorRotation();
        break;
      }
    }

    if (!bIsMatinee)
    {
//      int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
      // This should support multiple cameras
//      UGTCaptureComponent* CaptureComponent = FCaptureManager::Get().GetCamera(CameraId);
//      if (CaptureComponent == nullptr)
//      {
//        return FExecStatus::Error(FString::Printf(TEXT("Camera %d can not be found."), CameraId));
//      }
//      CameraLocation = CaptureComponent->GetComponentLocation();
//      CameraRotation = CaptureComponent->GetComponentRotation();

      APawn* Pawn = FUE4CVServer::Get().GetPawn();
      CameraLocation = Pawn->GetActorLocation();
      CameraRotation = Pawn->GetControlRotation();
    }

    FString Message = FString::Printf(TEXT("%.3f %.3f %.3f %.3f %.3f %.3f"),
                                      CameraLocation.X, CameraLocation.Y, CameraLocation.Z,
                                      CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);

    return FExecStatus::OK(Message);
  }
  return FExecStatus::Error("Number of arguments incorrect");
}

FExecStatus FCameraCommandHandler::SetCameraHorizontalFieldOfView(const TArray<FString>& Args)
{
    if (Args.Num() == 2)
    {
        int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
        if (CameraId != 0)
        {
        	return FExecStatus::Error("Setting field of view is only supported for camera 0");
        }

        float FieldOfView = FCString::Atof(*Args[1]);

        bool bIsMatinee = false;

        for (AActor* Actor : this->GetWorld()->GetCurrentLevel()->Actors)
        {
            // if (Actor && Actor->IsA(AMatineeActor::StaticClass())) // AMatineeActor is deprecated
            bool FoundCamera = false;
            if (Actor && Actor->IsA(ACameraActor::StaticClass()))
            {
                bIsMatinee = true;
                UCameraComponent* CameraComponent = Actor->FindComponentByClass<UCameraComponent>();
                if (CameraComponent != nullptr) {
                    UE_LOG(LogUnrealCV, Warning, TEXT("Setting field of view to: %f"), FieldOfView);
                    CameraComponent->SetFieldOfView(FieldOfView);
                    FoundCamera = true;
                    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
                    CameraManager->SetFOV(FieldOfView);
                    break;
                }
            }
        }

        UGTCaptureComponent* CaptureComponent = FCaptureManager::Get().GetCamera(CameraId);
        if (CaptureComponent == nullptr)
        {
          return FExecStatus::Error(FString::Printf(TEXT("Camera %d can not be found."), CameraId));
        }
            UGTCaptureComponent* GTCapturer = FCaptureManager::Get().GetCamera(CameraId);
            if (GTCapturer == nullptr)
            {
                return FExecStatus::Error(FString::Printf(TEXT("Invalid camera id %d"), CameraId));
            }
            GTCapturer->SetFOVAngle(FieldOfView);

        return FExecStatus::OK();
    }
    return FExecStatus::Error("Number of arguments incorrect");
}

FExecStatus FCameraCommandHandler::GetCameraHorizontalFieldOfView(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		bool bIsMatinee = false;

        float FieldOfView = 0.0f;

		/*for (AActor* Actor : this->GetWorld()->GetCurrentLevel()->Actors)
		{
			// if (Actor && Actor->IsA(AMatineeActor::StaticClass())) // AMatineeActor is deprecated
			if (Actor && Actor->IsA(ACameraActor::StaticClass()))
			{
				bIsMatinee = true;
                UCameraComponent* CameraComponent = Actor->FindComponentByClass<UCameraComponent>();
                if (CameraComponent != nullptr) {
                    FieldOfView = CameraComponent->FieldOfView;
                }
                UE_LOG(LogUnrealCV, Warning, TEXT("Got FOV from Matinee"));
				break;
			}
		}*/

		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		// APawn* Pawn = FUE4CVServer::Get().GetPawn();
		// CameraLocation = Pawn->GetActorLocation();
		UGTCaptureComponent* CaptureComponent = FCaptureManager::Get().GetCamera(CameraId);
		if (CaptureComponent == nullptr)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Camera %d can not be found."), CameraId));
		}
        UGTCaptureComponent* GTCapturer = FCaptureManager::Get().GetCamera(CameraId);
        if (GTCapturer == nullptr)
        {
            return FExecStatus::Error(FString::Printf(TEXT("Invalid camera id %d"), CameraId));
        }
        USceneCaptureComponent2D* SceneCaptureComponent = GTCapturer->GetCaptureComponent(TEXT("lit"));
        if (SceneCaptureComponent == nullptr)
        {
            return FExecStatus::Error(FString::Printf(TEXT("Unexpected error: Capture component not found.")));
        }
        FieldOfView = SceneCaptureComponent->FOVAngle;

		FString Message = FString::Printf(TEXT("%.3f"), FieldOfView);

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

TArray<uint8> FCameraCommandHandler::GetNpyBinaryUint8Data(const TArray<FString>& Args, const FString& ViewMode, int32 Channels)
{
	int32 CameraId = FCString::Atoi(*Args[0]);

	UGTCaptureComponent* GTCapturer = FCaptureManager::Get().GetCamera(CameraId);
	if (GTCapturer == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Invalid camera id %d"), CameraId);
		return TArray<uint8>();
	}

	TArray<uint8> ImgData = GTCapturer->CaptureNpyUint8(ViewMode, Channels);
	return ImgData;
}

FExecStatus FCameraCommandHandler::GetNpyBinaryUint8(const TArray<FString>& Args, const FString& ViewMode, int32 Channels)
{
	TArray<uint8> Data = GetNpyBinaryUint8Data(Args, ViewMode, Channels);
	return FExecStatus::Binary(Data);
}

TArray<uint8> FCameraCommandHandler::GetNpyBinaryFloat16Data(const TArray<FString>& Args, const FString& ViewMode, int32 Channels)
{
	int32 CameraId = FCString::Atoi(*Args[0]);

	UGTCaptureComponent* GTCapturer = FCaptureManager::Get().GetCamera(CameraId);
	if (GTCapturer == nullptr)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("Invalid camera id %d"), CameraId);
		return TArray<uint8>();
	}

	TArray<uint8> ImgData = GTCapturer->CaptureNpyFloat16(ViewMode, Channels);
	return ImgData;
}

FExecStatus FCameraCommandHandler::GetNpyBinaryFloat16(const TArray<FString>& Args, const FString& ViewMode, int32 Channels)
{
	TArray<uint8> Data = GetNpyBinaryFloat16Data(Args, ViewMode, Channels);
	return FExecStatus::Binary(Data);
}
