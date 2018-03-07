// Weichao Qiu @ 2017
// This is unrealcv command API for FusionSensor
#include "UnrealCVPrivate.h"
#include "CommandDispatcher.h"
#include "SensorHandler.h"
#include "FusionCamSensor.h"
#include "Serialization.h"
#include "StrFormatter.h"
#include "SensorBP.h"
#include "PlayerViewMode.h"

enum EFilenameType
{
	Png,
	Npy,
	Exr,
	Bmp,
	PngBinary,
	NpyBinary,
	BmpBinary,
	Invalid, // Unrecognized filename type
};

// FString WorldTypeStr(EWorldType::Type WorldType)
// {
// 	switch (WorldType)
// 	{
// 		case EWorldType::Editor : return TEXT("Editor");
// 		case EWorldType::EditorPreview: return TEXT("EditorPreview");
// 		case EWorldType::Game: return TEXT("Game");
// 		case EWorldType::GamePreview: return TEXT("GamePreview");
// 		case EWorldType::Inactive: return TEXT("Inactive");
// 		case EWorldType::None: return TEXT("None");
// 		case EWorldType::PIE: return TEXT("PIE");
// 	}
// 	return TEXT("Invalid");
// }

UFusionCamSensor* GetSensor(const TArray<FString>& Args, FExecStatus& Status)
{
	if (Args.Num() < 1)
	{
		Status = FExecStatus::Error("No sensor id is available");
		return nullptr;
	}
	int SensorId = FCString::Atoi(*Args[0]);
	UFusionCamSensor* FusionSensor = USensorBP::GetSensorById(SensorId);
	if (!IsValid(FusionSensor)) 
	{
		Status = FExecStatus::Error("Invalid sensor id");
		return nullptr;
	}
	return FusionSensor;
}


/** vget /sensors , List all sensors in the world */
FExecStatus GetSensorList(const TArray<FString>& Args)
{
	TArray<UFusionCamSensor*> GameWorldSensorList = USensorBP::GetFusionSensorList();

	FString StrSensorList;
	for (UFusionCamSensor* Sensor : GameWorldSensorList)
	{
		StrSensorList += FString::Printf(TEXT("%s "), *Sensor->GetName());
	}
	return FExecStatus::OK(StrSensorList);
}


FExecStatus GetSensorLocation(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, Status);
	if (!IsValid(FusionCamSensor)) return Status; 

	FStrFormatter Ar;
	FVector Location = FusionCamSensor->GetSensorLocation();
	Ar << Location;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus SetSensorLocation(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, Status);
	if (!IsValid(FusionCamSensor)) return Status; 

	// Should I set the component loction or the actor location?
	if (Args.Num() != 4) return FExecStatus::InvalidArgument; // ID, X, Y, Z

	float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
	FVector Location = FVector(X, Y, Z);

	// Check USceneComponent
	FusionCamSensor->SetSensorLocation(Location);

	return FExecStatus::OK();
}

FExecStatus GetSensorRotation(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, Status);
	if (!IsValid(FusionCamSensor)) return Status; 

	FRotator Rotation = FusionCamSensor->GetSensorRotation();
	FStrFormatter Ar;
	Ar << Rotation;

	return FExecStatus::OK(Ar.ToString());
}

FExecStatus SetSensorRotation(const TArray<FString>& Args)
{
	FExecStatus Status = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, Status);
	if (!IsValid(FusionCamSensor)) return Status; 

	if (Args.Num() != 4) return FExecStatus::InvalidArgument; // ID, X, Y, Z
	float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
	FRotator Rotator = FRotator(Pitch, Yaw, Roll);
	FusionCamSensor->SetSensorRotation(Rotator);

	return FExecStatus::OK();
}


// TODO: Move this to utility library
EFilenameType ParseFilenameType(const FString& Filename)
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
FExecStatus SerializeData(const TArray<FColor>& Data, int Width, int Height, const FString& Filename)
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

FExecStatus SerializeData(const TArray<FFloat16Color>& Data, int Width, int Height, const FString& Filename)
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

FExecStatus SerializeData(const TArray<float>& Data, int Width, int Height, const FString& Filename)
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
void SaveData(const TArray<T>& Data, int Width, int Height,
	const TArray<FString>& Args, FExecStatus& Status)
{
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


FExecStatus GetSensorLit(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetLitSlow(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus GetSensorLitFast(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetLit(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus GetSensorDepth(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<float> Data;
	int Width, Height;
	FusionCamSensor->GetDepth(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus GetSensorPlaneDepth(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FFloat16Color> Data;
	int Width, Height;
	FusionCamSensor->GetPlaneDepth(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus GetSensorVisDepth(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FFloat16Color> Data;
	int Width, Height;
	FusionCamSensor->GetVisDepth(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus GetSensorNormal(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetNormal(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus GetSensorObjMask(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetObjectMask(Data, Width, Height);
	// TODO: Remove this later for better performance
	for (int i = 0; i < Width * Height; i++)
	{
		Data[i].A = 255;
	}
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus GetSensorDepthStencil(const TArray<FString>& Args)
{
	FExecStatus ExecStatus = FExecStatus::OK();
	UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
	if (!IsValid(FusionCamSensor)) return ExecStatus; 

	TArray<FColor> Data;
	int Width, Height;
	FusionCamSensor->GetStencil(Data, Width, Height);
	SaveData(Data, Width, Height, Args, ExecStatus);
	return ExecStatus;
}

FExecStatus MoveTo(const TArray<FString>& Args)
{
	// FExecStatus ExecStatus = FExecStatus::OK();
	// UFusionCamSensor* FusionCamSensor = GetSensor(Args, ExecStatus);
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
	bool Success = FUE4CVServer::Get().GetPawn()->SetActorLocation(Location, Sweep, NULL, ETeleportType::TeleportPhysics);

	return FExecStatus::OK();
}


/** vget /screenshot [filename] */
FExecStatus GetScreenshot(const TArray<FString>& Args)
{
	FString Filename = Args[0];

	UWorld* World = FUE4CVServer::Get().GetGameWorld();
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

void FSensorHandler::RegisterCommands()
{
	CommandDispatcher->BindCommand(
		"vget /screenshot [str]",
		FDispatcherDelegate::CreateStatic(GetScreenshot),
		"Get screenshot");

	 CommandDispatcher->BindCommand(
		"vget /cameras",
		FDispatcherDelegate::CreateStatic(GetSensorList),
		"List all sensors in the scene");

	/*
	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/info",
		FDispatcherDelegate::CreateStatic(GetSensorInfo),
		"Get sensor information");
		*/

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/location",
		FDispatcherDelegate::CreateStatic(GetSensorLocation),
		"Get sensor location in world space"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/location [float] [float] [float]",
		FDispatcherDelegate::CreateStatic(SetSensorLocation),
		"Set sensor to location [x, y, z]"
	);

	/** This is different from SetLocation (which is teleport) */
	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/moveto [float] [float] [float]",
		FDispatcherDelegate::CreateStatic(MoveTo),
		"Move camera to location [x, y, z], will be blocked by objects"
	);

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/rotation",
		FDispatcherDelegate::CreateStatic(GetSensorRotation),
		"Get sensor rotation in world space"
	);

	CommandDispatcher->BindCommand(
		"vset /camera/[uint]/rotation [float] [float] [float]",
		FDispatcherDelegate::CreateStatic(SetSensorRotation),
		"Set rotation [pitch, yaw, roll] of camera [id]"
	);

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/lit [str]",
		FDispatcherDelegate::CreateStatic(GetSensorLit),
		"Get png binary data from lit sensor"
	);

	// TODO: Need to merge these two later
	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/lit_fast [str]",
		FDispatcherDelegate::CreateStatic(GetSensorLitFast),
		"Get png binary data from lit sensor"
	);

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/depth [str]",
		FDispatcherDelegate::CreateStatic(GetSensorDepth),
		"Get npy binary data from depth sensor");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/vis_depth [str]",
		FDispatcherDelegate::CreateStatic(GetSensorVisDepth),
		"Get npy binary data from vis depth sensor");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/plane_depth [str]",
		FDispatcherDelegate::CreateStatic(GetSensorPlaneDepth),
		"Get npy binary data from plane depth sensor");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/normal [str]",
		FDispatcherDelegate::CreateStatic(GetSensorNormal),
		"Get npy binary data from surface normal sensor");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/object_mask [str]",
		FDispatcherDelegate::CreateStatic(GetSensorObjMask),
		"Get npy binary data from depth sensor");

	CommandDispatcher->BindCommand(
		"vget /camera/[uint]/depth_stencil [str]",
		FDispatcherDelegate::CreateStatic(GetSensorDepthStencil),
		"Get depth stencil data as an alternative way for object mask");

	CommandDispatcher->BindCommand(
		"vset /viewmode [str]",
		FDispatcherDelegate::CreateRaw(&FPlayerViewMode::Get(), &FPlayerViewMode::SetMode),
		"Set ViewMode to (lit, normal, depth, object_mask)"
	);

	CommandDispatcher->BindCommand(
		"vget /viewmode",
		FDispatcherDelegate::CreateRaw(&FPlayerViewMode::Get(), &FPlayerViewMode::GetMode),
		"Get current ViewMode"
	);

	// Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetActorLocation);
	// Help = "Get actor location [x, y, z]";
	// CommandDispatcher->BindCommand("vget /actor/location", Cmd, Help);

	// Cmd = FDispatcherDelegate::CreateRaw(this, &FCameraCommandHandler::GetActorRotation);
	// Help = "Get actor rotation [pitch, yaw, roll]";
	// CommandDispatcher->BindCommand("vget /actor/rotation", Cmd, Help);
}