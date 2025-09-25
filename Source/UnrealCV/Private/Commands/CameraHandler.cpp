// Weichao Qiu @ 2017
// This is unrealcv command API for FusionSensor
#include "CameraHandler.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/Engine/Classes/GameFramework/Controller.h"
#include "Misc/Paths.h"

#include "CommandDispatcher.h"
#include "FusionCamSensor.h"
#include "Serialization.h"
#include "Utils/StrFormatter.h"
#include "PlayerViewMode.h"
#include "WorldController.h"
#include "ImageUtil.h"
#include "SensorBPLib.h"
#include "FusionCameraActor.h"

#include "UnrealcvStats.h"
#include "UnrealClient.h"

DECLARE_CYCLE_STAT(TEXT("FCameraHandler::GetCameraLit"), STAT_GetCameraLit, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("FCameraHandler::SaveData"), STAT_SaveData, STATGROUP_UnrealCV);

UFusionCamSensor* FCameraHandler::GetCamera(const TArray<FString>& Args, FExecStatus& Status)
{
	if (Args.Num() < 1)
	{
		FString Msg = TEXT("No sensor id is available");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		Status = FExecStatus::Error(Msg);
		return nullptr;
	}
	int SensorId = FCString::Atoi(*Args[0]);
	UFusionCamSensor* FusionSensor = USensorBPLib::GetSensorById(SensorId);
	if (!IsValid(FusionSensor)) 
	{
		FString Msg = TEXT("Invalid sensor id");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);
		Status = FExecStatus::Error(Msg);
		return nullptr;
	}
	return FusionSensor;
}


/** vget /sensors , List all sensors in the world */
FExecStatus FCameraHandler::GetCameraList(const TArray<FString>& Args)
{
	TArray<UFusionCamSensor*> GameWorldSensorList = USensorBPLib::GetFusionSensorList();

	FString StrSensorList;
	for (UFusionCamSensor* Sensor : GameWorldSensorList)
	{
		StrSensorList += FString::Printf(TEXT("%s "), *Sensor->GetName());
	}
	return FExecStatus::OK(StrSensorList);
}


FExecStatus FCameraHandler::GetCameraLocation(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return Status; 

	FStrFormatter Ar;
	FVector Location = FusionCamSensor->GetSensorLocation();
	Ar << Location;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus FCameraHandler::SetCameraLocation(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return Status; 

	// Should I set the component loction or the actor location?
	if (Args.Num() != 4) return FExecStatus::InvalidArgument; // ID, X, Y, Z

	float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
	FVector Location = FVector(X, Y, Z);

	if (Args[0] == "0")
	{
		// Note: For camera 0, we want to change the player location

		bool Sweep = false;
		// Note: If sweep is true, the object can not move through another object
		// Note: It will check invalid location and move back a bit.
		APawn* Pawn = FUnrealcvServer::Get().GetPawn();
		if (!IsValid(Pawn))
		{
			UE_LOG(LogTemp, Warning, TEXT("The Pawn of the scene is invalid."));
			return FExecStatus::InvalidArgument;
		}
		Pawn->SetActorLocation(Location, Sweep, NULL, ETeleportType::TeleportPhysics);
	}
	else
	{
		FusionCamSensor->SetSensorLocation(Location);
	}

	return FExecStatus::OK();
}

FExecStatus FCameraHandler::GetCameraRotation(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return Status; 

	FRotator Rotation = FusionCamSensor->GetSensorRotation();
	FStrFormatter Ar;
	Ar << Rotation;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus FCameraHandler::SetCameraRotation(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return Status; 

	if (Args.Num() != 4) return FExecStatus::InvalidArgument; // ID, X, Y, Z
	float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
	FRotator Rotator = FRotator(Pitch, Yaw, Roll);

	// Note: For camera 0, we want to change the player rotation
	if (Args[0] == "0")
	{
		APawn* Pawn = FUnrealcvServer::Get().GetPawn();
		if (!IsValid(Pawn))
		{
			UE_LOG(LogTemp, Warning, TEXT("The Pawn of the scene is invalid."));
			return FExecStatus::InvalidArgument;
		}
		AController* Controller = Pawn->GetController();
		if (!IsValid(Controller))
		{
			UE_LOG(LogTemp, Warning, TEXT("The Controller of the Pawn is invalid."));
			return FExecStatus::InvalidArgument;
		}
		Controller->ClientSetRotation(Rotator); // Teleport action
	}
	else
	{
		FusionCamSensor->SetSensorRotation(Rotator);
	}

	return FExecStatus::OK();
}


// TODO: Move this to utility library
EFilenameType FCameraHandler::ParseFilenameType(const FString& Filename)
{
	bool bIncludeDot = false;
	FString FileExtension = FPaths::GetExtension(Filename);
	FileExtension.ToLowerInline();

	// A hacky way to check whether the input is just a file extension
	int DotIndex;
	if (!Filename.FindChar('.', DotIndex)) FileExtension = Filename;

	if (FileExtension == Filename) // The filename only contains extension, which means the binary mode
	{
		if (FileExtension == TEXT("png")) return EFilenameType::PngBinary;
		if (FileExtension == TEXT("bmp")) return EFilenameType::BmpBinary;
		if (FileExtension == TEXT("npy")) return EFilenameType::NpyBinary;
	}
	else
	{
		if (FileExtension == TEXT("png")) return EFilenameType::Png;
		if (FileExtension == TEXT("bmp")) return EFilenameType::Bmp;
		if (FileExtension == TEXT("npy")) return EFilenameType::Npy;
		if (FileExtension == TEXT("exr")) return EFilenameType::Exr;
	}
	return EFilenameType::Invalid;
}

/** Serialize data according to filename format */
FExecStatus FCameraHandler::SerializeData(const TArray<FColor>& Data, int Width, int Height, const FString& Filename)
{
	static FImageUtil ImageUtil;
	EFilenameType FilenameType = ParseFilenameType(Filename);

	TArray<uint8> BinaryData;
	switch (FilenameType)
	{
	case EFilenameType::BmpBinary:
		ImageUtil.ConvertToBmp(Data, Width, Height, BinaryData);
		return FExecStatus::Binary(BinaryData);
	case EFilenameType::Bmp:
		ImageUtil.SaveBmpFile(Data, Width, Height, Filename);
		return FExecStatus::OK(Filename);
	case EFilenameType::PngBinary:
		ImageUtil.ConvertToPng(Data, Width, Height, BinaryData);
		return FExecStatus::Binary(BinaryData);
	case EFilenameType::Png:
		ImageUtil.SavePngFile(Data, Width, Height, Filename);
		return FExecStatus::OK(Filename);
	}
	return FExecStatus::Error(FString::Printf(TEXT("Invalid filename type, filename %s"), *Filename));
}

FExecStatus FCameraHandler::SerializeData(const TArray<FFloat16Color>& Data, int Width, int Height, const FString& Filename)
{
	static FImageUtil ImageUtil;
	EFilenameType FilenameType = ParseFilenameType(Filename);

	TArray<uint8> BinaryData;
	int Channel = Data.Num() / (Width * Height);
	switch (FilenameType)
	{
	case EFilenameType::NpyBinary:
		BinaryData = FSerializationUtils::Array2Npy(Data, Width, Height, Channel);
		return FExecStatus::Binary(BinaryData);
	case EFilenameType::Npy:
		BinaryData = FSerializationUtils::Array2Npy(Data, Width, Height, Channel);
		ImageUtil.SaveFile(BinaryData, Filename);
		return FExecStatus::OK(Filename);
	}
	return FExecStatus::Error(FString::Printf(TEXT("Invalid filename type, filename %s"), *Filename));
}

FExecStatus FCameraHandler::SerializeData(const TArray<float>& Data, int Width, int Height, const FString& Filename)
{
	static FImageUtil ImageUtil;
	EFilenameType FilenameType = ParseFilenameType(Filename);

	TArray<uint8> BinaryData;
	int Channel = Data.Num() / (Width * Height);
	switch (FilenameType)
	{
	case EFilenameType::NpyBinary:
		BinaryData = FSerializationUtils::Array2Npy(Data, Width, Height, Channel);
		return FExecStatus::Binary(BinaryData);
	case EFilenameType::Npy:
		BinaryData = FSerializationUtils::Array2Npy(Data, Width, Height, Channel);
		ImageUtil.SaveFile(BinaryData, Filename);
		return FExecStatus::OK(Filename);
	}
	return FExecStatus::Error(FString::Printf(TEXT("Invalid filename type, filename %s"), *Filename));
}

template<class T>
void FCameraHandler::SaveData(const TArray<T>& Data, int Width, int Height,
	const TArray<FString>& Args, FExecStatus& Status)
{
	SCOPE_CYCLE_COUNTER(STAT_SaveData);

	if (Args.Num() != 2)
	{
		Status = FExecStatus::Error("Filename can not be empty");
		return;
	}
	FString Filename = Args[1];
	if (Data.Num() == 0)
	{
		Status = FExecStatus::Error("Captured data is empty");
		return;
	}
	Status = SerializeData(Data, Width, Height, Filename);
	return;
}


FExecStatus FCameraHandler::GetCameraLit(const TArray<FString>& Args)
{
	SCOPE_CYCLE_COUNTER(STAT_GetCameraLit);

	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetLit(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus FCameraHandler::GetCameraDepth(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<float> Data;
	int Width, Height;
	FusionCamSensor->GetDepth(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}


FExecStatus FCameraHandler::GetCameraNormal(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetNormal(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus FCameraHandler::GetCameraFlow(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus;
	// FusionCamSensor->checkFusionSensors();

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetFlow(Data, Width, Height);
	if (Data.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("%s: Flow data is empty (if you are using old character/drone blueprints, you have to rebuild this project, because the flow sensor in FusionCamSensor is a component of the blueprint.)"), *FString(__FUNCTION__));
	}
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus FCameraHandler::GetCameraObjMask(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetSeg(Data, Width, Height);

	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus FCameraHandler::MoveTo(const TArray<FString>& Args)
{
	// FExecStatus ExecStatus = FExecStatus::OK();
	// UFusionCamSensor* FusionCamSensor = GetCamera(Args, ExecStatus);
	// if (!IsValid(FusionCamSensor)) return ExecStatus; 

	/** The API for Character, Pawn and Actor are different */
	if (Args.Num() != 4) // ID, X, Y, Z
	{
		return FExecStatus::InvalidArgument;
	}
	if (Args[0] != "0")
	{
		return FExecStatus::Error("MoveTo only supports the player camera with id 0");
	}

	float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
	FVector Location = FVector(X, Y, Z);

	bool Sweep = true;
	// if sweep is true, the object can not move through another object
	// Check invalid location and move back a bit.
	bool Success = FUnrealcvServer::Get().GetPawn()->SetActorLocation(Location, Sweep, NULL, ETeleportType::TeleportPhysics);

	return FExecStatus::OK();
}


/** vget /screenshot [filename] */
FExecStatus FCameraHandler::GetScreenshot(const TArray<FString>& Args)
{
	FString Filename = Args[0];

	UWorld* World = FUnrealcvServer::Get().GetWorld();
	UGameViewportClient* ViewportClient = World->GetGameViewport();

	bool bScreenshotSuccessful = false;
	FViewport* InViewport = ViewportClient->Viewport;
	ViewportClient->GetEngineShowFlags()->SetMotionBlur(false);
	FIntVector Size(InViewport->GetSizeXY().X, InViewport->GetSizeXY().Y, 0);

	TArray<FColor> Bitmap;
	bScreenshotSuccessful = GetViewportScreenShot(InViewport, Bitmap);
	// InViewport->ReadFloat16Pixels

	// Ensure that all pixels' alpha is set to 255
	for (auto& Color : Bitmap)
	{
		Color.A = 255;
	}
	// TODO: Need to blend alpha, a bit weird from screen.

	FExecStatus ExecStatus = SerializeData(Bitmap, Size.X, Size.Y, Filename);
	return ExecStatus;
}

FExecStatus FCameraHandler::SetPlayerViewMode(const TArray<FString>& Args)
{
	TWeakObjectPtr<AUnrealcvWorldController> WorldController = FUnrealcvServer::Get().WorldController;
	return WorldController->PlayerViewMode->SetMode(Args);
}

FExecStatus FCameraHandler::GetPlayerViewMode(const TArray<FString>& Args)
{
	TWeakObjectPtr<AUnrealcvWorldController> WorldController = FUnrealcvServer::Get().WorldController;
	return WorldController->PlayerViewMode->GetMode(Args);
}

FExecStatus FCameraHandler::GetFOV(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::InvalidArgument;
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;

	if (Args.Num() != 1) return FExecStatus::InvalidArgument; // ID

	float FOV = FusionCamSensor->GetSensorFOV();
	FString Res = FString::Printf(TEXT("%f"), FOV);
	return FExecStatus::OK(Res);
}

FExecStatus FCameraHandler::SetFOV(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::InvalidArgument;
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;

	if (Args.Num() != 2) return FExecStatus::InvalidArgument; // ID, FOV

	float FOV = FCString::Atof(*Args[1]);
	FusionCamSensor->SetSensorFOV(FOV);
	return FExecStatus::OK();
}

FExecStatus FCameraHandler::SpawnCamera(const TArray<FString>& Args)
{
	UWorld* GameWorld = FUnrealcvServer::Get().GetWorld();
	AActor* Actor = GameWorld->SpawnActor(AFusionCameraActor::StaticClass());
	if (IsValid(Actor))
	{
		return FExecStatus::OK(Actor->GetName());
	}
	else
	{
		return FExecStatus::Error("Failed to spawn actor");
	}
}

FExecStatus FCameraHandler::GetSize(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::InvalidArgument;
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;

	if (Args.Num() != 1) return FExecStatus::InvalidArgument; // ID

	int Width = FusionCamSensor->GetFilmWidth();
	int Height = FusionCamSensor->GetFilmHeight();
	FString Res = FString::Printf(TEXT("%d %d"), Width, Height);
	return FExecStatus::OK(Res);
}

FExecStatus FCameraHandler::SetSize(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::InvalidArgument;
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;

	if (Args.Num() != 3) return FExecStatus::InvalidArgument; // ID, Width, Height

	int Width = FCString::Atof(*Args[1]);
	int Height = FCString::Atof(*Args[2]);
	FusionCamSensor->SetFilmSize(Width, Height);
	return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetProjectionType(const TArray<FString>& Args)
{
	if (Args.Num() != 2) return FExecStatus::InvalidArgument;

	FExecStatus Status = FExecStatus::InvalidArgument;
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	FString ProjectionType = Args[1];
	if (ProjectionType.ToLower() == "perspective")
	{
		FusionCamSensor->SetProjectionType(ECameraProjectionMode::Type::Perspective);
		return FExecStatus::OK();
	}
	else if (ProjectionType.ToLower() == "orthographic")
	{
		FusionCamSensor->SetProjectionType(ECameraProjectionMode::Type::Orthographic);
		return FExecStatus::OK();
	}
	else
	{
		FString ErrorMsg = FString::Printf(TEXT("Can not support camera mode %s, available options are perspective and orthographic"), *ProjectionType);
		return FExecStatus::Error(ErrorMsg);
	}
}

FExecStatus FCameraHandler::SetOrthoWidth(const TArray<FString>& Args)
{
	if (Args.Num() != 2) return FExecStatus::InvalidArgument;

	FExecStatus Status = FExecStatus::InvalidArgument;
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);

	int OrthoWidth = FCString::Atof(*Args[1]);
	FusionCamSensor->SetOrthoWidth(OrthoWidth);
	return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetExposureMethod(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
	UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
	if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
	if (Args.Num() != 2) return FExecStatus::InvalidArgument; // exposure value
	FString ExposureType = Args[1];
	if (ExposureType.ToLower() == "histogram")
    {
        FusionCamSensor->SetExposureMethod(EAutoExposureMethod::AEM_Histogram);
        return FExecStatus::OK();
    }
    else if  (ExposureType.ToLower() == "basic")
    {
        FusionCamSensor->SetExposureMethod(EAutoExposureMethod::AEM_Basic);
        return FExecStatus::OK();
    }
    else if  (ExposureType.ToLower() == "manual")
    {
        FusionCamSensor->SetExposureMethod(EAutoExposureMethod::AEM_Manual);
        return FExecStatus::OK();
    }
    else
    {
        FString ErrorMsg = FString::Printf(TEXT("Can not support auto exposure mode %s, available options are true and false"), *ExposureType);
        return FExecStatus::Error(ErrorMsg);
    }
}

FExecStatus FCameraHandler::SetLitSource(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 2) return FExecStatus::InvalidArgument;
    FString LitSource = Args[1];
    if (LitSource.ToLower() == "ftc_hdr")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_FinalToneCurveHDR);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "fc_hdr")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_FinalColorHDR);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "sc_hdr")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_SceneColorHDR);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "scna_hdr")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_SceneColorHDRNoAlpha);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "ldr")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_FinalColorLDR);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "base")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_BaseColor);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "color_depth")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_SceneDepth);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "scene_depth")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_SceneDepth);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "device_depth")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_DeviceDepth);
        return FExecStatus::OK();
    }
    else if (LitSource.ToLower() == "normal")
    {
        FusionCamSensor->SetLitCaptureSource(ESceneCaptureSource::SCS_Normal);
        return FExecStatus::OK();
    }
    else
    {
        FString ErrorMsg = FString::Printf(TEXT("Can not support lit source %s, available options are ftc_hdr, fc_hdr, sc_hdr, scna_hdr, ldr, base, color_depth, scene_depth, device_depth, normal"), *LitSource);
        return FExecStatus::Error(ErrorMsg);
    }
}

FExecStatus FCameraHandler::SetReflectionMethod(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 2) return FExecStatus::InvalidArgument;
    FString ReflectionMethod = Args[1];
    if (ReflectionMethod.ToLower() == "none")
    {
        FusionCamSensor->SetReflectionMethod(EReflectionMethod::Type::None);
        return FExecStatus::OK();
    }
    else if (ReflectionMethod.ToLower() == "lumen")
    {
        FusionCamSensor->SetReflectionMethod(EReflectionMethod::Type::Lumen);
        return FExecStatus::OK();
    }
    else if (ReflectionMethod.ToLower() == "screen_space")
    {
        FusionCamSensor->SetReflectionMethod(EReflectionMethod::Type::ScreenSpace);
        return FExecStatus::OK();
    }
    else
    {
        FString ErrorMsg = FString::Printf(TEXT("Can not support reflection method %s, available options are none, lumen, screen_space."), *ReflectionMethod);
        return FExecStatus::Error(ErrorMsg);
    }
}

FExecStatus FCameraHandler::SetGlobalIlluminationMethod(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 2) return FExecStatus::InvalidArgument;
    FString IlluminationMethod = Args[1];
    if (IlluminationMethod.ToLower() == "none")
    {
        FusionCamSensor->SetGlobalIlluminationMethod(EDynamicGlobalIlluminationMethod::Type::None);
        return FExecStatus::OK();
    }
    else if (IlluminationMethod.ToLower() == "lumen")
    {
        FusionCamSensor->SetGlobalIlluminationMethod(EDynamicGlobalIlluminationMethod::Type::Lumen);
        return FExecStatus::OK();
    }
    else if (IlluminationMethod.ToLower() == "screen_space")
    {
        FusionCamSensor->SetGlobalIlluminationMethod(EDynamicGlobalIlluminationMethod::Type::ScreenSpace);
        return FExecStatus::OK();
    }
    else if (IlluminationMethod.ToLower() == "plugin")
    {
        FusionCamSensor->SetGlobalIlluminationMethod(EDynamicGlobalIlluminationMethod::Type::Plugin);
        return FExecStatus::OK();
    }
    else
    {
        FString ErrorMsg = FString::Printf(TEXT("Can not support global illumination method %s, available options are none, lumen, screen_space, plugin."), *IlluminationMethod);
        return FExecStatus::Error(ErrorMsg);
    }
}

FExecStatus FCameraHandler::SetExposureBias(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 2) return FExecStatus::InvalidArgument; // exposure value
    float ExposureBias = FCString::Atof(*Args[1]);
    FusionCamSensor->SetExposureBias(ExposureBias);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetAutoExposureSpeed(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 3) return FExecStatus::InvalidArgument; // exposure value
    float SpeedDown = FCString::Atof(*Args[1]);
    float SpeedUp = FCString::Atof(*Args[2]);
    FusionCamSensor->SetAutoExposureSpeed(SpeedDown, SpeedUp);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetAutoExposureBrightness(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 3) return FExecStatus::InvalidArgument;
    float MinBrightness = FCString::Atof(*Args[1]);
    float MaxBrightness = FCString::Atof(*Args[2]);
    FusionCamSensor->SetAutoExposureBrightness(MinBrightness, MaxBrightness);
    return FExecStatus::OK();
}


FExecStatus FCameraHandler::SetApplyPhysicalCameraExposure(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 2) return FExecStatus::InvalidArgument;
    int ApplyPhysicalCameraExposure = FCString::Atoi(*Args[1]);
    FusionCamSensor->SetApplyPhysicalCameraExposure(ApplyPhysicalCameraExposure);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetMotionBlurParams(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 5) return FExecStatus::InvalidArgument; // motion blur amount, max, per object, fps
    float MotionBlurAmount = FCString::Atof(*Args[1]);
    float MotionBlurMax = FCString::Atof(*Args[2]);
    float MotionBlurPerObject = FCString::Atof(*Args[3]);
    int MotionBlurFPS = FCString::Atoi(*Args[4]);
    FusionCamSensor->SetMotionBlurParams(MotionBlurAmount, MotionBlurMax, MotionBlurPerObject, MotionBlurFPS);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetFocalParams(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor)) return FExecStatus::InvalidArgument;
    if (Args.Num() != 3) return FExecStatus::InvalidArgument; // exposure value
    float FocalDistance = FCString::Atof(*Args[1]);
    float FocalRange = FCString::Atof(*Args[2]);
    FusionCamSensor->SetFocalParams(FocalDistance, FocalRange);
    return FExecStatus::OK();
}


void FCameraHandler::RegisterCommands()
{
	CommandDispatcher->BindCommand(
		"vget /screenshot [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetScreenshot),
		"Get screenshot");

	CommandDispatcher->BindCommand(
		"vget /cameras",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraList),
		"List all sensors in the scene");

	CommandDispatcher->BindCommand(
		"vset /cameras/spawn",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SpawnCamera),
		"Spawn a new camera actor in the scene");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/location",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraLocation),
		"Get sensor location in world space"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/location [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCameraLocation),
		"Set sensor to location [x, y, z]"
	);

	/** This is different from SetLocation (which is teleport) */
	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/moveto [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::MoveTo),
		"Move camera to location [x, y, z], will be blocked by objects"
	);

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/rotation",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraRotation),
		"Get sensor rotation in world space"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/rotation [float] [float] [float]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCameraRotation),
		"Set rotation [pitch, yaw, roll] of camera [id]"
	);

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/lit [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraLit),
		"Get png binary data from lit sensor"
	);

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/depth [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraDepth),
		"Get npy binary data from depth sensor");


	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/normal [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraNormal),
		"Get npy binary data from surface normal sensor");

	// CommandDispatcher->BindCommand(
	// 	"vget /camera/[uint]/flow [str]",
	// 	FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraFlow),
	// 	"Get npy binary data from optical flow sensor");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/optical_flow [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraFlow),
		"Get npy binary data from optical flow sensor");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/object_mask [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraObjMask),
		"Get object mask from camera sensor");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/seg [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraObjMask),
		"Get object mask from camera sensor");

	CommandDispatcher->BindCommand(
		"vset /viewmode [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetPlayerViewMode),
		"Set ViewMode to (lit, normal, depth, object_mask)"
	);

	CommandDispatcher->BindCommand(
		"vget /viewmode",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetPlayerViewMode),
		"Get current ViewMode"
	);

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/fov",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetFOV),
		"Get FOV"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/fov [float]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetFOV),
		"Set FOV"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/size [uint] [uint]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetSize),
		"Set Camera Film Size"
	);

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/size",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetSize),
		"Get Camera Film Size"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/ortho_width [float]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetOrthoWidth),
		"Set ortho width of the camera"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/projection_type [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetProjectionType),
		"Set camera projection type"
	);

	CommandDispatcher->BindCommand(
        "vset /camera/[uint]/lit_source [str]",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetLitSource),
        "Set the capture source of the lit camera"
    );

    CommandDispatcher->BindCommand(
		"vset /camera/[uint]/reflection [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetReflectionMethod),
		"Set camera reflection method: None, Lumen, ScreenSpace"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/illumination [str]",
		FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetGlobalIlluminationMethod),
		"Set camera global illumination method: None, Lumen, ScreenSpace, Plugin,"
	);

	CommandDispatcher->BindCommand(
	    "vset /camera/[uint]/exposure_method [str]",
	    FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetExposureMethod),
	    "Set camera exposure method"
	);

	CommandDispatcher->BindCommand(
        "vset /camera/[uint]/exposure_bias [float]",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetExposureBias),
        "Set camera exposure bias"
    );

    CommandDispatcher->BindCommand(
        "vset /camera/[uint]/auto_speed [float] [float]",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetAutoExposureSpeed),
        "Set camera auto-exposure speed down and speed up"
    );

    CommandDispatcher->BindCommand(
        "vset /camera/[uint]/auto_brightness [float] [float]",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetAutoExposureBrightness),
        "Set camera auto-exposure min max brightness"
    );

    CommandDispatcher->BindCommand(
        "vset /camera/[uint]/physical_exposure [uint]",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetApplyPhysicalCameraExposure),
        "Set camera apply physical camera exposure"
    );

    CommandDispatcher->BindCommand(
        "vset /camera/[uint]/motion_blur [float] [float] [float] [uint]",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetMotionBlurParams),
        "Set camera motion blur amount, max, per object, fps"
    );

    CommandDispatcher->BindCommand(
        "vset /camera/[uint]/focal [float] [float]",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetFocalParams),
        "Set camera focus distance and range"
    );
}
