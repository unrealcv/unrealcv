// Weichao Qiu @ 2017
// This is unrealcv command API for FusionSensor
#include "CameraHandler.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/Engine/Classes/GameFramework/Controller.h"
#include "Misc/Paths.h"

#include "CommandDispatcher.h"
#include "UnrealcvServer.h"
#include "FusionCamSensor.h"
#include "CineCameraComponent.h"
#include "Utils/UObjectUtils.h"
#include "Serialization.h"
#include "Utils/StrFormatter.h"
#include "PlayerViewMode.h"
#include "WorldController.h"
#include "ImageUtil.h"
#include "SensorBPLib.h"
#include "FusionCameraActor.h"
#include "Utils/SharedMemoryManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"

#include "UnrealcvStats.h"
#include "UnrealcvLog.h"
#include "UnrealClient.h"

DECLARE_CYCLE_STAT(TEXT("FCameraHandler::GetCameraLit"), STAT_GetCameraLit, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("FCameraHandler::SaveData"), STAT_SaveData, STATGROUP_UnrealCV);

namespace
{
FString SerializeJsonObject(const TSharedRef<FJsonObject>& JsonObject)
{
    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    FJsonSerializer::Serialize(JsonObject, Writer);
    return Output;
}

TArray<TSharedPtr<FJsonValue>> MakeJsonNumberArray(std::initializer_list<double> Values)
{
    TArray<TSharedPtr<FJsonValue>> Result;
    for (double Value : Values)
    {
        Result.Add(MakeShared<FJsonValueNumber>(Value));
    }
    return Result;
}

FString SharedImageResponse(const FUnrealCVSharedMemoryView& View, const FString& Modality, int32 Width, int32 Height,
                            const FString& DType, const FString& Layout, const FString& ChannelOrder,
                            const TArray<TSharedPtr<FJsonValue>>& Shape, uint64 Frame)
{
    TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
    Json->SetStringField(TEXT("transport"), TEXT("windows_shared_memory"));
    Json->SetStringField(TEXT("modality"), Modality);
    Json->SetStringField(TEXT("name"), View.Name);
    Json->SetNumberField(TEXT("num_bytes"), static_cast<double>(View.NumBytes));
    Json->SetNumberField(TEXT("version"), static_cast<double>(View.Version));
    Json->SetNumberField(TEXT("offset_bytes"), View.OffsetBytes);
    Json->SetArrayField(TEXT("shape"), Shape);
    Json->SetStringField(TEXT("dtype"), DType);
    Json->SetStringField(TEXT("layout"), Layout);
    if (!ChannelOrder.IsEmpty())
        Json->SetStringField(TEXT("channel_order"), ChannelOrder);
    Json->SetNumberField(TEXT("width"), Width);
    Json->SetNumberField(TEXT("height"), Height);
    Json->SetNumberField(TEXT("frame"), static_cast<double>(Frame));
    Json->SetStringField(
        TEXT("lifetime"),
        TEXT("valid until the next capture writes the same shared memory name or the UnrealCV server exits"));
    return SerializeJsonObject(Json);
}

template <typename T>
FExecStatus WriteSharedImage(const TArray<T>& Data, int Width, int Height, const FString& LogicalName,
                             const FString& Modality, const FString& DType, const FString& Layout,
                             const FString& Channels, const TArray<TSharedPtr<FJsonValue>>& Shape, uint64 Frame)
{
    if (Width <= 0 || Height <= 0 || Data.Num() != Width * Height)
        return FExecStatus::Error(FString::Printf(TEXT("Invalid %s dimensions or data size: %dx%d, samples=%d"),
                                                  *Modality, Width, Height, Data.Num()));
    FUnrealCVSharedMemoryView View;
    FString Error;
    if (!FUnrealCVSharedMemoryManager::Get().WriteBytes(LogicalName, Data.GetData(),
                                                        static_cast<int64>(Data.Num()) * sizeof(T), View, Error))
        return FExecStatus::Error(Error);
    return FExecStatus::OK(SharedImageResponse(View, Modality, Width, Height, DType, Layout, Channels, Shape, Frame));
}

template <typename TEnum> bool ResolveOption(const TMap<FString, TEnum>& Options, const FString& Input, TEnum& OutValue)
{
    const FString Key = Input.ToLower();
    if (const TEnum* Found = Options.Find(Key))
    {
        OutValue = *Found;
        return true;
    }
    return false;
}
} // anonymous namespace

UFusionCamSensor* FCameraHandler::GetCamera(const TArray<FString>& Args, FExecStatus& Status)
{
    if (Args.Num() < 1)
    {
        FString Msg = TEXT("No sensor id is available");
        UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *Msg);
        Status = FExecStatus::Error(Msg);
        return nullptr;
    }
    int SensorId = FCString::Atoi(*Args[0]);
    UFusionCamSensor* FusionSensor = USensorBPLib::GetSensorById(SensorId);
    if (!IsValid(FusionSensor))
    {
        FString Msg = TEXT("Invalid sensor id");
        UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *Msg);
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
    if (!IsValid(FusionCamSensor))
        return Status;

    FStrFormatter Ar;
    FVector Location = FusionCamSensor->GetSensorLocation();
    Ar << Location;

    return FExecStatus::OK(Ar.ToString());
}

FExecStatus FCameraHandler::SetCameraLocation(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::OK();
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return Status;

    // Should I set the component loction or the actor location?
    if (Args.Num() != 4)
        return FExecStatus::InvalidArgument; // ID, X, Y, Z

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
            UE_LOG(LogUnrealCV, Warning, TEXT("The Pawn of the scene is invalid."));
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
    if (!IsValid(FusionCamSensor))
        return Status;

    FRotator Rotation = FusionCamSensor->GetSensorRotation();
    FStrFormatter Ar;
    Ar << Rotation;

    return FExecStatus::OK(Ar.ToString());
}

FExecStatus FCameraHandler::SetCameraRotation(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::OK();
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return Status;

    if (Args.Num() != 4)
        return FExecStatus::InvalidArgument; // ID, X, Y, Z
    float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
    FRotator Rotator = FRotator(Pitch, Yaw, Roll);

    // Note: For camera 0, we want to change the player rotation
    if (Args[0] == "0")
    {
        APawn* Pawn = FUnrealcvServer::Get().GetPawn();
        if (!IsValid(Pawn))
        {
            UE_LOG(LogUnrealCV, Warning, TEXT("The Pawn of the scene is invalid."));
            return FExecStatus::InvalidArgument;
        }
        AController* Controller = Pawn->GetController();
        if (!IsValid(Controller))
        {
            UE_LOG(LogUnrealCV, Warning, TEXT("The Controller of the Pawn is invalid."));
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
    FString FileExtension = FPaths::GetExtension(Filename);
    FileExtension.ToLowerInline();

    // A hacky way to check whether the input is just a file extension
    int DotIndex;
    if (!Filename.FindChar('.', DotIndex))
        FileExtension = Filename;

    if (FileExtension == Filename) // The filename only contains extension, which means the binary mode
    {
        if (FileExtension == TEXT("png"))
            return EFilenameType::PngBinary;
        if (FileExtension == TEXT("bmp"))
            return EFilenameType::BmpBinary;
        if (FileExtension == TEXT("npy"))
            return EFilenameType::NpyBinary;
    }
    else
    {
        if (FileExtension == TEXT("png"))
            return EFilenameType::Png;
        if (FileExtension == TEXT("bmp"))
            return EFilenameType::Bmp;
        if (FileExtension == TEXT("npy"))
            return EFilenameType::Npy;
        if (FileExtension == TEXT("exr"))
            return EFilenameType::Exr;
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

FExecStatus FCameraHandler::SerializeData(const TArray<FFloat16Color>& Data, int Width, int Height,
                                          const FString& Filename)
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

template <class T>
void FCameraHandler::SaveData(const TArray<T>& Data, int Width, int Height, const TArray<FString>& Args,
                              FExecStatus& Status)
{
    SCOPE_CYCLE_COUNTER(STAT_SaveData);

    if (Args.Num() != 2)
    {
        Status = FExecStatus::Error("Filename can not be empty");
        return;
    }
    const FString& Filename = Args[1];
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
    if (!IsValid(FusionCamSensor))
        return ExecStatus;

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
    if (!IsValid(FusionCamSensor))
        return ExecStatus;

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
    if (!IsValid(FusionCamSensor))
        return ExecStatus;

    TArray<FColor> Data;
    int Width, Height;
    FusionCamSensor->GetNormal(Data, Width, Height);
    SaveData(Data, Width, Height, Args, ExecStatus);
    return ExecStatus;
}

FExecStatus FCameraHandler::GetCameraLitShared(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::OK();
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor))
        return Status;
    TArray<FColor> Data;
    int Width = 0, Height = 0;
    Sensor->GetLit(Data, Width, Height);
    static uint64 Frame = 0;
    return WriteSharedImage(Data, Width, Height, FString::Printf(TEXT("camera_%s_lit_bgra8"), *Args[0]), TEXT("lit"),
                            TEXT("uint8"), TEXT("HWC"), TEXT("BGRA"),
                            MakeJsonNumberArray({double(Height), double(Width), 4.0}), ++Frame);
}

FExecStatus FCameraHandler::GetCameraDepthShared(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::OK();
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor))
        return Status;
    TArray<float> Data;
    int Width = 0, Height = 0;
    Sensor->GetDepth(Data, Width, Height);
    static uint64 Frame = 0;
    FExecStatus Result = WriteSharedImage(
        Data, Width, Height, FString::Printf(TEXT("camera_%s_depth_float32"), *Args[0]), TEXT("depth"), TEXT("float32"),
        TEXT("HW"), TEXT(""), MakeJsonNumberArray({double(Height), double(Width)}), ++Frame);
    return Result;
}

FExecStatus FCameraHandler::GetCameraNormalShared(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::OK();
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor))
        return Status;
    TArray<FColor> Data;
    int Width = 0, Height = 0;
    Sensor->GetNormal(Data, Width, Height);
    static uint64 Frame = 0;
    return WriteSharedImage(Data, Width, Height, FString::Printf(TEXT("camera_%s_normal_bgra8"), *Args[0]),
                            TEXT("normal"), TEXT("uint8"), TEXT("HWC"), TEXT("BGRA"),
                            MakeJsonNumberArray({double(Height), double(Width), 4.0}), ++Frame);
}

FExecStatus FCameraHandler::GetCameraSegShared(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::OK();
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor))
        return Status;
    TArray<FColor> Data;
    int Width = 0, Height = 0;
    Sensor->GetSeg(Data, Width, Height);
    static uint64 Frame = 0;
    return WriteSharedImage(Data, Width, Height, FString::Printf(TEXT("camera_%s_object_mask_bgra8"), *Args[0]),
                            TEXT("object_mask"), TEXT("uint8"), TEXT("HWC"), TEXT("BGRA"),
                            MakeJsonNumberArray({double(Height), double(Width), 4.0}), ++Frame);
}

FExecStatus FCameraHandler::GetCameraFlow(const TArray<FString>& Args)
{
    FExecStatus ExecStatus = FExecStatus::OK();
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, ExecStatus);
    if (!IsValid(FusionCamSensor))
        return ExecStatus;

    TArray<FColor> Data;
    int Width, Height;
    FusionCamSensor->GetFlow(Data, Width, Height);
    if (Data.Num() == 0)
    {
        UE_LOG(LogUnrealCV, Error,
               TEXT("%s: Flow data is empty (if you are using old character/drone blueprints, you have to rebuild this "
                    "project, because the flow sensor in FusionCamSensor is a component of the blueprint.)"),
               *FString(__FUNCTION__));
    }
    SaveData(Data, Width, Height, Args, ExecStatus);
    return ExecStatus;
}

FExecStatus FCameraHandler::GetCameraObjMask(const TArray<FString>& Args)
{
    FExecStatus ExecStatus = FExecStatus::OK();
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, ExecStatus);
    if (!IsValid(FusionCamSensor))
        return ExecStatus;

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

    constexpr bool bSweep = true;
    // If sweep is true, the object cannot move through another object.
    APawn* Pawn = FUnrealcvServer::Get().GetPawn();
    if (!IsValid(Pawn))
    {
        return FExecStatus::Error(TEXT("The pawn is invalid"));
    }
    Pawn->SetActorLocation(Location, bSweep, nullptr, ETeleportType::TeleportPhysics);

    return FExecStatus::OK();
}

/** vget /screenshot [filename] */
FExecStatus FCameraHandler::GetScreenshot(const TArray<FString>& Args)
{
    if (Args.Num() < 1)
    {
        return FExecStatus::InvalidArgument;
    }
    const FString Filename = Args[0];

    UWorld* World = FUnrealcvServer::Get().GetWorld();
    if (!IsValid(World))
    {
        return FExecStatus::Error(TEXT("No valid world"));
    }
    UGameViewportClient* ViewportClient = World->GetGameViewport();
    if (!IsValid(ViewportClient) || !ViewportClient->Viewport)
    {
        return FExecStatus::Error(TEXT("No valid game viewport"));
    }

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
    const auto WorldController = FUnrealcvServer::Get().GetWorldController();
    if (!WorldController.IsValid() || !WorldController->PlayerViewMode)
    {
        return FExecStatus::Error(TEXT("WorldController or PlayerViewMode is not available"));
    }
    return WorldController->PlayerViewMode->SetMode(Args);
}

FExecStatus FCameraHandler::GetPlayerViewMode(const TArray<FString>& Args)
{
    const auto WorldController = FUnrealcvServer::Get().GetWorldController();
    if (!WorldController.IsValid() || !WorldController->PlayerViewMode)
    {
        return FExecStatus::Error(TEXT("WorldController or PlayerViewMode is not available"));
    }
    return WorldController->PlayerViewMode->GetMode(Args);
}

FExecStatus FCameraHandler::GetFOV(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;

    if (Args.Num() != 1)
        return FExecStatus::InvalidArgument; // ID

    float FOV = FusionCamSensor->GetSensorFOV();
    FString Res = FString::Printf(TEXT("%f"), FOV);
    return FExecStatus::OK(Res);
}

FExecStatus FCameraHandler::SetFOV(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;

    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument; // ID, FOV

    float FOV = FCString::Atof(*Args[1]);
    FusionCamSensor->SetSensorFOV(FOV);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::GetCineCamera(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 1) return FExecStatus::InvalidArgument;
    UCineCameraComponent* Cine = Sensor->GetCineCameraComponent();
    if (!IsValid(Cine)) return FExecStatus::Error(TEXT("Cine camera model is not available"));

    const FCameraFilmbackSettings& Filmback = Cine->Filmback;
    const FCameraLensSettings& Lens = Cine->LensSettings;
    const FCameraFocusSettings& Focus = Cine->FocusSettings;
    const AActor* TrackingActor = Focus.TrackingFocusSettings.ActorToTrack.Get();
    FString FocusMode = TEXT("unknown");
    switch (Focus.FocusMethod)
    {
    case ECameraFocusMethod::DoNotOverride: FocusMode = TEXT("do_not_override"); break;
    case ECameraFocusMethod::Manual: FocusMode = TEXT("manual"); break;
    case ECameraFocusMethod::Tracking: FocusMode = TEXT("tracking"); break;
    case ECameraFocusMethod::Disable: FocusMode = TEXT("disable"); break;
    default: break;
    }

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetBoolField(TEXT("enabled"), Sensor->IsCineCameraEnabled());

    TSharedRef<FJsonObject> FilmbackObject = MakeShared<FJsonObject>();
    FilmbackObject->SetNumberField(TEXT("sensor_width_mm"), Filmback.SensorWidth);
    FilmbackObject->SetNumberField(TEXT("sensor_height_mm"), Filmback.SensorHeight);
    FilmbackObject->SetNumberField(TEXT("sensor_offset_x_mm"), Filmback.SensorHorizontalOffset);
    FilmbackObject->SetNumberField(TEXT("sensor_offset_y_mm"), Filmback.SensorVerticalOffset);
    Root->SetObjectField(TEXT("filmback"), FilmbackObject);

    TSharedRef<FJsonObject> LensObject = MakeShared<FJsonObject>();
    LensObject->SetNumberField(TEXT("focal_length_mm"), Cine->CurrentFocalLength);
    LensObject->SetNumberField(TEXT("aperture_fstop"), Cine->CurrentAperture);
    LensObject->SetNumberField(TEXT("min_focal_length_mm"), Lens.MinFocalLength);
    LensObject->SetNumberField(TEXT("max_focal_length_mm"), Lens.MaxFocalLength);
    LensObject->SetNumberField(TEXT("min_fstop"), Lens.MinFStop);
    LensObject->SetNumberField(TEXT("max_fstop"), Lens.MaxFStop);
    LensObject->SetNumberField(TEXT("minimum_focus_distance_mm"), Lens.MinimumFocusDistance);
    LensObject->SetNumberField(TEXT("squeeze_factor"), Lens.SqueezeFactor);
    LensObject->SetNumberField(TEXT("diaphragm_blade_count"), Lens.DiaphragmBladeCount);
    Root->SetObjectField(TEXT("lens"), LensObject);

    TSharedRef<FJsonObject> FocusObject = MakeShared<FJsonObject>();
    FocusObject->SetStringField(TEXT("mode"), FocusMode);
    FocusObject->SetNumberField(TEXT("manual_distance_cm"), Focus.ManualFocusDistance);
    FocusObject->SetBoolField(TEXT("smooth"), Focus.bSmoothFocusChanges);
    FocusObject->SetNumberField(TEXT("smoothing_speed"), Focus.FocusSmoothingInterpSpeed);
    FocusObject->SetNumberField(TEXT("offset_cm"), Focus.FocusOffset);
    FocusObject->SetStringField(TEXT("tracking_actor"), TrackingActor ? TrackingActor->GetName() : TEXT(""));
    FocusObject->SetArrayField(TEXT("tracking_offset_cm"), MakeJsonNumberArray({
        Focus.TrackingFocusSettings.RelativeOffset.X,
        Focus.TrackingFocusSettings.RelativeOffset.Y,
        Focus.TrackingFocusSettings.RelativeOffset.Z}));
    Root->SetObjectField(TEXT("focus"), FocusObject);

    TSharedRef<FJsonObject> CropObject = MakeShared<FJsonObject>();
    CropObject->SetNumberField(TEXT("aspect_ratio"), Cine->CropSettings.AspectRatio);
    CropObject->SetNumberField(TEXT("overscan"), Cine->Overscan);
    CropObject->SetBoolField(TEXT("crop_overscan"), Cine->bCropOverscan);
    CropObject->SetBoolField(TEXT("scale_resolution_with_overscan"), Cine->bScaleResolutionWithOverscan);
    Root->SetObjectField(TEXT("crop"), CropObject);

    TSharedRef<FJsonObject> ExposureObject = MakeShared<FJsonObject>();
    ExposureObject->SetNumberField(TEXT("iso"), Cine->PostProcessSettings.CameraISO);
    ExposureObject->SetNumberField(TEXT("shutter_speed_reciprocal"), Cine->PostProcessSettings.CameraShutterSpeed);
    ExposureObject->SetBoolField(TEXT("apply_physical_exposure"), Cine->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure);
    Root->SetObjectField(TEXT("exposure"), ExposureObject);

    TSharedRef<FJsonObject> NearClipObject = MakeShared<FJsonObject>();
    NearClipObject->SetBoolField(TEXT("override"), Cine->bOverride_CustomNearClippingPlane);
    NearClipObject->SetNumberField(TEXT("distance_cm"), Cine->CustomNearClippingPlane);
    Root->SetObjectField(TEXT("near_clip"), NearClipObject);
    Root->SetNumberField(TEXT("horizontal_fov_degrees"), Cine->GetHorizontalFieldOfView());
    Root->SetNumberField(TEXT("vertical_fov_degrees"), Cine->GetVerticalFieldOfView());
    return FExecStatus::OK(SerializeJsonObject(Root));
}

FExecStatus FCameraHandler::GetCineCameraEnabled(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 1) return FExecStatus::InvalidArgument;
    return FExecStatus::OK(Sensor->IsCineCameraEnabled() ? TEXT("1") : TEXT("0"));
}

FExecStatus FCameraHandler::SetCineCameraEnabled(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 2) return FExecStatus::InvalidArgument;
    Sensor->SetCineCameraEnabled(FCString::Atoi(*Args[1]) != 0);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetCineFilmback(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 5) return FExecStatus::InvalidArgument;
    Sensor->SetCineFilmback(FCString::Atof(*Args[1]), FCString::Atof(*Args[2]), FCString::Atof(*Args[3]), FCString::Atof(*Args[4]));
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetCineLens(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 3) return FExecStatus::InvalidArgument;
    Sensor->SetCineLens(FCString::Atof(*Args[1]), FCString::Atof(*Args[2]));
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetCineFocus(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 2) return FExecStatus::InvalidArgument;
    Sensor->SetCineFocusDistance(FCString::Atof(*Args[1]));
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetCineLensSettings(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 8) return FExecStatus::InvalidArgument;
    Sensor->SetCineLensSettings(FCString::Atof(*Args[1]), FCString::Atof(*Args[2]), FCString::Atof(*Args[3]),
        FCString::Atof(*Args[4]), FCString::Atof(*Args[5]), FCString::Atof(*Args[6]), FCString::Atoi(*Args[7]));
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetCineFocusMode(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 5) return FExecStatus::InvalidArgument;
    if (!Sensor->SetCineFocusMode(Args[1], FCString::Atoi(*Args[2]) != 0, FCString::Atof(*Args[3]), FCString::Atof(*Args[4])))
    {
        return FExecStatus::Error(TEXT("Focus mode must be manual, tracking, disable, none, or do_not_override"));
    }
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetCineTrackingFocus(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 5) return FExecStatus::InvalidArgument;
    AActor* TargetActor = GetActorById(FUnrealcvServer::Get().GetWorld(), Args[1]);
    if (!IsValid(TargetActor)) return FExecStatus::Error(FString::Printf(TEXT("Tracking actor '%s' was not found"), *Args[1]));
    return Sensor->SetCineTrackingFocus(TargetActor, FVector(FCString::Atof(*Args[2]), FCString::Atof(*Args[3]), FCString::Atof(*Args[4])))
        ? FExecStatus::OK() : FExecStatus::Error(TEXT("Failed to configure tracking focus"));
}

FExecStatus FCameraHandler::SetCineCrop(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 5) return FExecStatus::InvalidArgument;
    Sensor->SetCineCrop(FCString::Atof(*Args[1]), FCString::Atof(*Args[2]), FCString::Atoi(*Args[3]) != 0, FCString::Atoi(*Args[4]) != 0);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetCineNearClip(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 3) return FExecStatus::InvalidArgument;
    Sensor->SetCineNearClip(FCString::Atoi(*Args[1]) != 0, FCString::Atof(*Args[2]));
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetCineExposure(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 4) return FExecStatus::InvalidArgument;
    Sensor->SetCineExposure(FCString::Atof(*Args[1]), FCString::Atof(*Args[2]), FCString::Atoi(*Args[3]) != 0);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::GetCineIntrinsics(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* Sensor = GetCamera(Args, Status);
    if (!IsValid(Sensor) || Args.Num() != 1) return FExecStatus::InvalidArgument;
    UCineCameraComponent* Cine = Sensor->GetCineCameraComponent();
    if (!IsValid(Cine)) return FExecStatus::Error(TEXT("Cine camera model is not available"));

    FMinimalViewInfo ViewInfo;
    Cine->GetCameraView(0.0f, ViewInfo);
    const int32 Width = FMath::Max(1, FMath::RoundToInt(Sensor->GetFilmWidth()));
    const int32 Height = FMath::Max(1, FMath::RoundToInt(Sensor->GetFilmHeight()));
    ViewInfo.AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
    ViewInfo.bConstrainAspectRatio = false;
    const FMatrix Projection = ViewInfo.CalculateProjectionMatrix();
    const double Fx = Projection.M[0][0] * Width * 0.5;
    const double Fy = Projection.M[1][1] * Height * 0.5;
    const double Cx = (1.0 + Projection.M[2][0]) * Width * 0.5;
    const double Cy = (1.0 - Projection.M[2][1]) * Height * 0.5;

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("width"), Width);
    Root->SetNumberField(TEXT("height"), Height);
    Root->SetNumberField(TEXT("fx"), Fx);
    Root->SetNumberField(TEXT("fy"), Fy);
    Root->SetNumberField(TEXT("cx"), Cx);
    Root->SetNumberField(TEXT("cy"), Cy);
    Root->SetNumberField(TEXT("horizontal_fov_degrees"), Cine->GetHorizontalFieldOfView());
    Root->SetNumberField(TEXT("vertical_fov_degrees"), Cine->GetVerticalFieldOfView());
    Root->SetArrayField(TEXT("projection_offset"), MakeJsonNumberArray({ViewInfo.OffCenterProjectionOffset.X, ViewInfo.OffCenterProjectionOffset.Y}));
    Root->SetArrayField(TEXT("projection_matrix"), MakeJsonNumberArray({
        Projection.M[0][0], Projection.M[0][1], Projection.M[0][2], Projection.M[0][3],
        Projection.M[1][0], Projection.M[1][1], Projection.M[1][2], Projection.M[1][3],
        Projection.M[2][0], Projection.M[2][1], Projection.M[2][2], Projection.M[2][3],
        Projection.M[3][0], Projection.M[3][1], Projection.M[3][2], Projection.M[3][3]}));
    return FExecStatus::OK(SerializeJsonObject(Root));
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
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;

    if (Args.Num() != 1)
        return FExecStatus::InvalidArgument; // ID

    int Width = FusionCamSensor->GetFilmWidth();
    int Height = FusionCamSensor->GetFilmHeight();
    FString Res = FString::Printf(TEXT("%d %d"), Width, Height);
    return FExecStatus::OK(Res);
}

FExecStatus FCameraHandler::SetSize(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;

    if (Args.Num() != 3)
        return FExecStatus::InvalidArgument; // ID, Width, Height

    const int32 Width = FCString::Atoi(*Args[1]);
    const int32 Height = FCString::Atoi(*Args[2]);
    FusionCamSensor->SetFilmSize(Width, Height);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetProjectionType(const TArray<FString>& Args)
{
    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument;

    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return Status;
    static const TMap<FString, ECameraProjectionMode::Type> ProjectionOptions = {
        {TEXT("perspective"), ECameraProjectionMode::Type::Perspective},
        {TEXT("orthographic"), ECameraProjectionMode::Type::Orthographic},
    };

    ECameraProjectionMode::Type ProjectionMode;
    if (!ResolveOption(ProjectionOptions, Args[1], ProjectionMode))
    {
        return FExecStatus::Error(FString::Printf(
            TEXT("Can not support camera mode %s, available options are perspective and orthographic"), *Args[1]));
    }
    FusionCamSensor->SetProjectionType(ProjectionMode);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetOrthoWidth(const TArray<FString>& Args)
{
    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument;

    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return Status;

    const float OrthoWidth = FCString::Atof(*Args[1]);
    FusionCamSensor->SetOrthoWidth(OrthoWidth);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetExposureMethod(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument; // exposure value
    static const TMap<FString, EAutoExposureMethod> ExposureOptions = {
        {TEXT("histogram"), EAutoExposureMethod::AEM_Histogram},
        {TEXT("basic"), EAutoExposureMethod::AEM_Basic},
        {TEXT("manual"), EAutoExposureMethod::AEM_Manual},
    };

    EAutoExposureMethod ExposureMethod;
    if (!ResolveOption(ExposureOptions, Args[1], ExposureMethod))
    {
        return FExecStatus::Error(FString::Printf(
            TEXT("Can not support auto exposure mode %s, available options are histogram, basic and manual"),
            *Args[1]));
    }
    FusionCamSensor->SetExposureMethod(ExposureMethod);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetLitSource(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument;

    static const TMap<FString, ESceneCaptureSource> LitSourceOptions = {
        {TEXT("ftc_hdr"), ESceneCaptureSource::SCS_FinalToneCurveHDR},
        {TEXT("fc_hdr"), ESceneCaptureSource::SCS_FinalColorHDR},
        {TEXT("sc_hdr"), ESceneCaptureSource::SCS_SceneColorHDR},
        {TEXT("scna_hdr"), ESceneCaptureSource::SCS_SceneColorHDRNoAlpha},
        {TEXT("ldr"), ESceneCaptureSource::SCS_FinalColorLDR},
        {TEXT("base"), ESceneCaptureSource::SCS_BaseColor},
        {TEXT("color_depth"), ESceneCaptureSource::SCS_SceneDepth},
        {TEXT("scene_depth"), ESceneCaptureSource::SCS_SceneDepth},
        {TEXT("device_depth"), ESceneCaptureSource::SCS_DeviceDepth},
        {TEXT("normal"), ESceneCaptureSource::SCS_Normal},
    };

    ESceneCaptureSource CaptureSource;
    if (!ResolveOption(LitSourceOptions, Args[1], CaptureSource))
    {
        return FExecStatus::Error(
            FString::Printf(TEXT("Can not support lit source %s, available options are ftc_hdr, fc_hdr, sc_hdr, "
                                 "scna_hdr, ldr, base, color_depth, scene_depth, device_depth, normal"),
                            *Args[1]));
    }
    FusionCamSensor->SetLitCaptureSource(CaptureSource);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetReflectionMethod(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument;

    static const TMap<FString, EReflectionMethod::Type> ReflectionOptions = {
        {TEXT("none"), EReflectionMethod::Type::None},
        {TEXT("lumen"), EReflectionMethod::Type::Lumen},
        {TEXT("screen_space"), EReflectionMethod::Type::ScreenSpace},
    };

    EReflectionMethod::Type ReflectionMethod;
    if (!ResolveOption(ReflectionOptions, Args[1], ReflectionMethod))
    {
        return FExecStatus::Error(FString::Printf(
            TEXT("Can not support reflection method %s, available options are none, lumen, screen_space."), *Args[1]));
    }
    FusionCamSensor->SetReflectionMethod(ReflectionMethod);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetGlobalIlluminationMethod(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument;

    static const TMap<FString, EDynamicGlobalIlluminationMethod::Type> IlluminationOptions = {
        {TEXT("none"), EDynamicGlobalIlluminationMethod::Type::None},
        {TEXT("lumen"), EDynamicGlobalIlluminationMethod::Type::Lumen},
        {TEXT("screen_space"), EDynamicGlobalIlluminationMethod::Type::ScreenSpace},
        {TEXT("plugin"), EDynamicGlobalIlluminationMethod::Type::Plugin},
    };

    EDynamicGlobalIlluminationMethod::Type IlluminationMethod;
    if (!ResolveOption(IlluminationOptions, Args[1], IlluminationMethod))
    {
        return FExecStatus::Error(FString::Printf(TEXT("Can not support global illumination method %s, available "
                                                       "options are none, lumen, screen_space, plugin."),
                                                  *Args[1]));
    }
    FusionCamSensor->SetGlobalIlluminationMethod(IlluminationMethod);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetExposureBias(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument; // exposure value
    float ExposureBias = FCString::Atof(*Args[1]);
    FusionCamSensor->SetExposureBias(ExposureBias);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetAutoExposureSpeed(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 3)
        return FExecStatus::InvalidArgument; // exposure value
    float SpeedDown = FCString::Atof(*Args[1]);
    float SpeedUp = FCString::Atof(*Args[2]);
    FusionCamSensor->SetAutoExposureSpeed(SpeedDown, SpeedUp);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetAutoExposureBrightness(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 3)
        return FExecStatus::InvalidArgument;
    float MinBrightness = FCString::Atof(*Args[1]);
    float MaxBrightness = FCString::Atof(*Args[2]);
    FusionCamSensor->SetAutoExposureBrightness(MinBrightness, MaxBrightness);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetApplyPhysicalCameraExposure(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 2)
        return FExecStatus::InvalidArgument;
    int ApplyPhysicalCameraExposure = FCString::Atoi(*Args[1]);
    FusionCamSensor->SetApplyPhysicalCameraExposure(ApplyPhysicalCameraExposure);
    return FExecStatus::OK();
}

FExecStatus FCameraHandler::SetMotionBlurParams(const TArray<FString>& Args)
{
    FExecStatus Status = FExecStatus::InvalidArgument;
    UFusionCamSensor* FusionCamSensor = GetCamera(Args, Status);
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 5)
        return FExecStatus::InvalidArgument; // motion blur amount, max, per object, fps
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
    if (!IsValid(FusionCamSensor))
        return FExecStatus::InvalidArgument;
    if (Args.Num() != 3)
        return FExecStatus::InvalidArgument; // exposure value
    float FocalDistance = FCString::Atof(*Args[1]);
    float FocalRange = FCString::Atof(*Args[2]);
    FusionCamSensor->SetFocalParams(FocalDistance, FocalRange);
    return FExecStatus::OK();
}

void FCameraHandler::RegisterCommands()
{
    CommandDispatcher->BindCommand("vget /screenshot [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetScreenshot),
                                   "Get screenshot");

    CommandDispatcher->BindCommand("vget /cameras",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraList),
                                   "List all sensors in the scene");

    CommandDispatcher->BindCommand("vset /cameras/spawn",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SpawnCamera),
                                   "Spawn a new camera actor in the scene");

    CommandDispatcher->BindCommand("vget /camera/[uint]/location",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraLocation),
                                   "Get sensor location in world space");

    CommandDispatcher->BindCommand("vset /camera/[uint]/location [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCameraLocation),
                                   "Set sensor to location [x, y, z]");

    /** This is different from SetLocation (which is teleport) */
    CommandDispatcher->BindCommand("vset /camera/[uint]/moveto [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::MoveTo),
                                   "Move camera to location [x, y, z], will be blocked by objects");

    CommandDispatcher->BindCommand("vget /camera/[uint]/rotation",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraRotation),
                                   "Get sensor rotation in world space");

    CommandDispatcher->BindCommand("vset /camera/[uint]/rotation [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCameraRotation),
                                   "Set rotation [pitch, yaw, roll] of camera [id]");

    CommandDispatcher->BindCommand("vget /camera/[uint]/lit [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraLit),
                                   "Get png binary data from lit sensor");

    CommandDispatcher->BindCommand("vget /camera/[uint]/depth [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraDepth),
                                   "Get npy binary data from depth sensor");

    CommandDispatcher->BindCommand("vget /camera/[uint]/normal [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraNormal),
                                   "Get npy binary data from surface normal sensor");

    CommandDispatcher->BindCommand("vget /camera/[uint]/lit_shared",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraLitShared),
                                   "Get lit image through Windows shared memory");
    CommandDispatcher->BindCommand("vget /camera/[uint]/depth_shared",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraDepthShared),
                                   "Get depth image through Windows shared memory");
    CommandDispatcher->BindCommand("vget /camera/[uint]/normal_shared",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraNormalShared),
                                   "Get normal image through Windows shared memory");

    // CommandDispatcher->BindCommand(
    // 	"vget /camera/[uint]/flow [str]",
    // 	FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraFlow),
    // 	"Get npy binary data from optical flow sensor");

    CommandDispatcher->BindCommand("vget /camera/[uint]/optical_flow [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraFlow),
                                   "Get npy binary data from optical flow sensor");

    CommandDispatcher->BindCommand("vget /camera/[uint]/object_mask [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraObjMask),
                                   "Get object mask from camera sensor");

    CommandDispatcher->BindCommand("vget /camera/[uint]/seg [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraObjMask),
                                   "Get object mask from camera sensor");
    CommandDispatcher->BindCommand("vget /camera/[uint]/object_mask_shared",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraSegShared),
                                   "Get object mask through Windows shared memory");
    CommandDispatcher->BindCommand("vget /camera/[uint]/seg_shared",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCameraSegShared),
                                   "Get segmentation through Windows shared memory");

    CommandDispatcher->BindCommand("vset /viewmode [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetPlayerViewMode),
                                   "Set ViewMode to (lit, normal, depth, object_mask)");

    CommandDispatcher->BindCommand("vget /viewmode",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetPlayerViewMode),
                                   "Get current ViewMode");

    CommandDispatcher->BindCommand("vget /camera/[uint]/fov",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetFOV), "Get FOV");

    CommandDispatcher->BindCommand("vset /camera/[uint]/fov [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetFOV), "Set FOV");

    CommandDispatcher->BindCommand("vget /camera/[uint]/cine",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCineCamera),
                                   "Get physical Cine camera settings as JSON");

    CommandDispatcher->BindCommand("vget /camera/[uint]/cine/enabled",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCineCameraEnabled),
                                   "Get whether the physical Cine camera path is enabled");

    CommandDispatcher->BindCommand("vget /camera/[uint]/cine/intrinsics",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetCineIntrinsics),
                                   "Get Cine intrinsics, projection offsets, FOVs, and projection matrix as JSON");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/enabled [uint]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineCameraEnabled),
                                   "Enable or disable the physical Cine camera path");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/filmback [float] [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineFilmback),
                                   "Set Cine filmback width, height, horizontal offset, and vertical offset in millimeters");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/lens [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineLens),
                                   "Set Cine focal length in millimeters and aperture in f-stops");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/lens_settings [float] [float] [float] [float] [float] [float] [uint]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineLensSettings),
                                   "Set Cine lens limits, minimum focus distance, squeeze factor, and blade count");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/focus [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineFocus),
                                   "Set Cine manual focus distance in centimeters");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/focus_mode [str] [uint] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineFocusMode),
                                   "Set Cine focus mode, smoothing state, smoothing speed, and focus offset");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/focus_tracking [str] [float] [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineTrackingFocus),
                                   "Set Cine tracking-focus actor and relative offset");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/crop [float] [float] [uint] [uint]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineCrop),
                                   "Set Cine crop aspect ratio, overscan, crop state, and resolution-scaling state");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/near_clip [uint] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineNearClip),
                                   "Enable or disable the Cine near clipping plane and set its distance in centimeters");

    CommandDispatcher->BindCommand("vset /camera/[uint]/cine/exposure [float] [float] [uint]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetCineExposure),
                                   "Set Cine ISO, shutter-speed reciprocal, and physical-exposure state");

    CommandDispatcher->BindCommand("vset /camera/[uint]/size [uint] [uint]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetSize),
                                   "Set Camera Film Size");

    CommandDispatcher->BindCommand("vget /camera/[uint]/size",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::GetSize),
                                   "Get Camera Film Size");

    CommandDispatcher->BindCommand("vset /camera/[uint]/ortho_width [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetOrthoWidth),
                                   "Set ortho width of the camera");

    CommandDispatcher->BindCommand("vset /camera/[uint]/projection_type [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetProjectionType),
                                   "Set camera projection type");

    CommandDispatcher->BindCommand("vset /camera/[uint]/lit_source [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetLitSource),
                                   "Set the capture source of the lit camera");

    CommandDispatcher->BindCommand("vset /camera/[uint]/reflection [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetReflectionMethod),
                                   "Set camera reflection method: None, Lumen, ScreenSpace");

    CommandDispatcher->BindCommand("vset /camera/[uint]/illumination [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetGlobalIlluminationMethod),
                                   "Set camera global illumination method: None, Lumen, ScreenSpace, Plugin,");

    CommandDispatcher->BindCommand("vset /camera/[uint]/exposure_method [str]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetExposureMethod),
                                   "Set camera exposure method");

    CommandDispatcher->BindCommand("vset /camera/[uint]/exposure_bias [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetExposureBias),
                                   "Set camera exposure bias");

    CommandDispatcher->BindCommand("vset /camera/[uint]/auto_speed [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetAutoExposureSpeed),
                                   "Set camera auto-exposure speed down and speed up");

    CommandDispatcher->BindCommand("vset /camera/[uint]/auto_brightness [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetAutoExposureBrightness),
                                   "Set camera auto-exposure min max brightness");

    CommandDispatcher->BindCommand(
        "vset /camera/[uint]/physical_exposure [uint]",
        FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetApplyPhysicalCameraExposure),
        "Set camera apply physical camera exposure");

    CommandDispatcher->BindCommand("vset /camera/[uint]/motion_blur [float] [float] [float] [uint]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetMotionBlurParams),
                                   "Set camera motion blur amount, max, per object, fps");

    CommandDispatcher->BindCommand("vset /camera/[uint]/focal [float] [float]",
                                   FDispatcherDelegate::CreateRaw(this, &FCameraHandler::SetFocalParams),
                                   "Set camera focus distance and range");
}
