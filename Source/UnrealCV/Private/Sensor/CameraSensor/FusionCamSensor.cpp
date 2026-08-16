// Weichao Qiu @ 2018
#include "FusionCamSensor.h"
#include "CineCameraComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "ImageUtil.h"
#include "Serialization.h"
#include "UnrealcvLog.h"
#include "UnrealcvServer.h"

// Sensors included in FusionSensor
#include "LitCamSensor.h"
#include "DepthCamSensor.h"
#include "NormalCamSensor.h"
#include "AnnotationCamSensor.h"
#include "FlowCamSensor.h"

UFusionCamSensor::UFusionCamSensor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FString ComponentName;
	ComponentName = FString::Printf(TEXT("%s_%s"), *this->GetName(), TEXT("PreviewCamera"));
	PreviewCamera = CreateDefaultSubobject<UCameraComponent>(*ComponentName);
	PreviewCamera->SetupAttachment(this);

	ComponentName = FString::Printf(TEXT("%s_%s"), *this->GetName(), TEXT("DepthCamSensor"));
	DepthCamSensor = CreateDefaultSubobject<UDepthCamSensor>(*ComponentName);
	FusionSensors.Add(DepthCamSensor);

	ComponentName = FString::Printf(TEXT("%s_%s"), *this->GetName(), TEXT("NormalCamSensor"));
	NormalCamSensor = CreateDefaultSubobject<UNormalCamSensor>(*ComponentName);
	FusionSensors.Add(NormalCamSensor);

	ComponentName = FString::Printf(TEXT("%s_%s"), *this->GetName(), TEXT("AnnotationCamSensor"));
	AnnotationCamSensor = CreateDefaultSubobject<UAnnotationCamSensor>(*ComponentName);
	FusionSensors.Add(AnnotationCamSensor);

	ComponentName = FString::Printf(TEXT("%s_%s"), *this->GetName(), TEXT("LitCamSensor"));
	LitCamSensor = CreateDefaultSubobject<ULitCamSensor>(*ComponentName);
	FusionSensors.Add(LitCamSensor);

	// FlowCamSensor is created later to avoid template mismatch issues
	// Unhandled Exception: EXCEPTION_ACCESS_VIOLATION reading address 0x0000000000000008 UnrealEditor_Engine UnrealEditor_UnrealCV!AUnrealcvWorldController::OpenLevel() 
	// FlowCamSensor = nullptr;
	ComponentName = FString::Printf(TEXT("%s_%s"), *this->GetName(), TEXT("FlowCamSensor"));
	FlowCamSensor = CreateDefaultSubobject<UFlowCamSensor>(*ComponentName);
	// FlowCamSensor = NewObject<UFlowCamSensor>(this, UFlowCamSensor::StaticClass()); /*NewObject with empty name can't be used to create default subobjects*/
	FusionSensors.Add(FlowCamSensor);

	// The config loading code should not be placed into the ctor, otherwise it will break the copy behavior
	const FServerConfig& Config = FUnrealcvServer::Get().GetConfig();
	FilmWidth = Config.Width == 0 ? 640 : Config.Width;
	FilmHeight = Config.Height == 0 ? 480 : Config.Height;
	FOV = Config.FOV == 0 ? 90 : Config.FOV;
	// Note: If FOV == 0, the render will give FMod assert error.
	// Need to call update functions after copy operator (in BeginPlay), here just sets value

	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (IsValid(Sensor))
		{
			if (Sensor != FlowCamSensor) { Sensor->SetupAttachment(this); }
		}
		else
		{
			// UE_LOG(LogUnrealCV, Warning, TEXT("Invalid sensor is found in the ctor of FusionCamSensor"));
			UE_LOG(LogUnrealCV, Error, TEXT("Invalid sensor is found in the ctor of FusionCamSensor"));
		}
	}
	// SetFilmSize(FilmWidth, FilmHeight); // This should not not be done in CTOR.
	// print pointers in FusionSensors
}

void UFusionCamSensor::checkFusionSensors()
{
	for (int i = 0; i < FusionSensors.Num(); i++)
	{
		if (!IsValid(FusionSensors[i]))
		{
			// UE_LOG(LogUnrealCV, Error, TEXT("UFusionCamSensor::checkFusionSensors: Invalid sensor id=%d p=%p, total=%d, this=%p"), i, FusionSensors[i], FusionSensors.Num(), this);
			UE_LOG(LogTemp, Warning, TEXT("Sensor %d within FusionCamSensor is invalid."), i);
		}
	}
}

void UFusionCamSensor::BeginPlay()
{
	Super::BeginPlay();

	// LogOutputDevice: Error: Ensure condition failed: false  [File:D:\build\++UE5\Sync\Engine\Source\Runtime\Engine\Private\Components\SceneComponent.cpp] [Line: 2104]
	// LogOutputDevice: Error: Template Mismatch during attachment. Attaching instanced component to template component. Parent 'FusionCamSensor_GEN_VARIABLE' (Owner 'None') Self 'FusionCamSensor_GEN_VARIABLE_FlowCamSensor' (Owner 'BP_Drone01_C_1').
	// So we have to attach FlowCamSensor after the actor is spawned, in BeginPlay.
	// If we put this in the ctor, I think all the blueprints have to be rebuild to fix this bug.
	// Howerver, because we put the AttachToComponent here, we can no longger use editor to adjust the FlowCam transform in blueprint.
	if (IsValid(FlowCamSensor))
	{
		// Ensure condition failed: !bRegistered  [File:D:\build\++UE5\Sync\Engine\Source\Runtime\Engine\Private\Components\SceneComponent.cpp] [Line: 1958]
		// SetupAttachment should only be used to initialize AttachParent and AttachSocketName for a future AttachToComponent. Once a component is registered you must use AttachToComponent. Owner [/Game/SuburbNeighborhoodHousePack/Maps/SuburbNeighborhood_Day.SuburbNeighborhood_Day:PersistentLevel.BP_Character_C_1], InParent [FusionCamSensor], InSocketName [None]
		// FlowCamSensor->SetupAttachment(this);

		FlowCamSensor->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	    const FTransform LitTransform = LitCamSensor->GetComponentTransform();
	    FlowCamSensor->SetWorldTransform(LitTransform);
		// const FTransform LitRelativeTransform = LitCamSensor->GetRelativeTransform();
		// FlowCamSensor->SetRelativeTransform(LitRelativeTransform);
	}
	else
	{
		UE_LOG(LogUnrealCV, Error, TEXT("FlowCamSensor is not initialized. Flow data will be empty."));
	}

	SetFilmSize(FilmWidth, FilmHeight);
	SetSensorFOV(FOV);
}

// void UFusionCamSensor::OnRegister()
// {
// 	Super::OnRegister();

// 	for (UBaseCameraSensor* Sensor : FusionSensors)
// 	{
// 		if (IsValid(Sensor))
// 		{
// 			Sensor->RegisterComponent();
// 		}
// 		else
// 		{
// 			UE_LOG(LogUnrealCV, Warning, TEXT("Invalid sensor is found in the OnRegister of FusionCamSensor"));
// 		}
// 	}
// }

bool UFusionCamSensor::GetEditorPreviewInfo(float DeltaTime, FMinimalViewInfo& ViewOut)
{
	// From CameraComponent
	if (this->IsActive())
	{
		this->LitCamSensor->GetCameraView(DeltaTime, ViewOut);
		return true;
	}
	else
	{
		return false;
	}
}

// color
void UFusionCamSensor::GetLit(TArray<FColor>& LitData, int& Width, int& Height, ELitMode LitMode)
{
	this->LitCamSensor->CaptureLit(LitData, Width, Height);
}

// depth
void UFusionCamSensor::GetDepth(TArray<float>& DepthData, int& Width, int& Height, EDepthMode DepthMode)
{
	this->DepthCamSensor->CaptureDepth(DepthData, Width, Height);
}

// normal
void UFusionCamSensor::GetNormal(TArray<FColor>& NormalData, int& Width, int& Height)
{
	this->NormalCamSensor->CaptureNormal(NormalData, Width, Height);
}

// optical flow
void UFusionCamSensor::GetFlow(TArray<FColor>& FlowData, int& Width, int& Height)
{
	if (!FlowCamSensor)
	{
		UE_LOG(LogUnrealCV, Error, TEXT("FlowCamSensor is not initialized. Flow data will be empty."));
		FlowData.Empty();
		Width = 0;
		Height = 0;
		return;
	}
	// this->FlowCamSensor->CaptureFlow(FlowData, Width, Height);
	this->FlowCamSensor->Capture(FlowData, Width, Height);
}

// Semantic
void UFusionCamSensor::GetSeg(TArray<FColor>& ObjMaskData, int& Width, int& Height, ESegMode SegMode)
{
	this->AnnotationCamSensor->CaptureSeg(ObjMaskData, Width, Height);
}

FVector UFusionCamSensor::GetSensorLocation()
{
	return this->GetComponentLocation(); // World space
}

FRotator UFusionCamSensor::GetSensorRotation()
{
	return this->GetComponentRotation(); // World space
}

void UFusionCamSensor::SetSensorLocation(FVector Location)
{
	this->SetWorldLocation(Location);
}

void UFusionCamSensor::SetSensorRotation(FRotator Rotator)
{
	this->SetWorldRotation(Rotator);
}

void UFusionCamSensor::SetFilmSize(int Width, int Height)
{
	this->FilmWidth = Width;
	this->FilmHeight = Height;
	if (Height == 0 || Width == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid film size %d x %d"), Width, Height);
		return;
	}

	// There are still bugs in compiled blueprints, I tried to fix them in the ctor, but it still fails.
	// So I have to manually init the texture target for FlowCamSensor.
	if (IsValid(FlowCamSensor))
	{
		FlowCamSensor->SetFilmSize(Width, Height);
	}
	else
	{
		UE_LOG(LogUnrealCV, Error, TEXT("FlowCamSensor is not initialized. Flow data will be empty."));
	}

	for (int i = 0; i < FusionSensors.Num(); i++)
	{
		UBaseCameraSensor* Sensor = FusionSensors[i];
		if (IsValid(Sensor))
		{
			Sensor->SetFilmSize(FilmWidth, FilmHeight);
		}
		else
		{
			// UE_LOG(LogTemp, Error, TEXT("SetFilmSize: Sensor %d within FusionCamSensor is invalid. this: %p"), i, this);
			UE_LOG(LogTemp, Warning, TEXT("SetFilmSize: Sensor %d within FusionCamSensor is invalid."), i);
		}
	}
}

float UFusionCamSensor::GetSensorFOV()
{
	return this->LitCamSensor->GetFOV();
}

void UFusionCamSensor::SetSensorFOV(float fov)
{
	this->FOV = fov;
	for (UBaseCameraSensor* Sensor: FusionSensors)
	{
		if (IsValid(Sensor))
		{
			Sensor->SetFOV(fov);
		}
	}
}

UCineCameraComponent* UFusionCamSensor::GetCineCameraComponent() const
{
	return IsValid(LitCamSensor) ? LitCamSensor->GetCineCameraComponent() : nullptr;
}

TArray<UCineCameraComponent*> UFusionCamSensor::GetCineCameraComponents() const
{
	TArray<UCineCameraComponent*> Components;
	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (IsValid(Sensor) && IsValid(Sensor->GetCineCameraComponent()))
		{
			Components.Add(Sensor->GetCineCameraComponent());
		}
	}
	return Components;
}

void UFusionCamSensor::SetCineCameraEnabled(bool bEnabled)
{
	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (IsValid(Sensor))
		{
			Sensor->SetCineCameraEnabled(bEnabled);
		}
	}
}

bool UFusionCamSensor::IsCineCameraEnabled() const
{
	return IsValid(LitCamSensor) && LitCamSensor->IsCineCameraEnabled();
}

void UFusionCamSensor::SetCineFilmback(float SensorWidth, float SensorHeight, float HorizontalOffset, float VerticalOffset)
{
	SetCineCameraEnabled(true);
	FCameraFilmbackSettings Filmback;
	Filmback.SensorWidth = FMath::Max(SensorWidth, 0.001f);
	Filmback.SensorHeight = FMath::Max(SensorHeight, 0.001f);
	Filmback.SensorHorizontalOffset = HorizontalOffset;
	Filmback.SensorVerticalOffset = VerticalOffset;
	Filmback.RecalcSensorAspectRatio();
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		Cine->SetFilmback(Filmback);
	}
	FOV = GetSensorFOV();
}

void UFusionCamSensor::SetCineLens(float FocalLength, float Aperture)
{
	SetCineCameraEnabled(true);
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		Cine->SetCurrentFocalLength(FMath::Max(FocalLength, 0.001f));
		Cine->SetCurrentAperture(FMath::Max(Aperture, 0.1f));
	}
	FOV = GetSensorFOV();
}

void UFusionCamSensor::SetCineLensSettings(float MinFocalLength, float MaxFocalLength, float MinFStop, float MaxFStop,
	float MinimumFocusDistance, float SqueezeFactor, int32 DiaphragmBladeCount)
{
	SetCineCameraEnabled(true);
	FCameraLensSettings LensSettings;
	LensSettings.MinFocalLength = FMath::Max(MinFocalLength, 0.001f);
	LensSettings.MaxFocalLength = FMath::Max(MaxFocalLength, LensSettings.MinFocalLength);
	LensSettings.MinFStop = FMath::Max(MinFStop, 0.1f);
	LensSettings.MaxFStop = FMath::Max(MaxFStop, LensSettings.MinFStop);
	LensSettings.MinimumFocusDistance = FMath::Max(MinimumFocusDistance, 0.0f);
	LensSettings.SqueezeFactor = FMath::Clamp(SqueezeFactor, 1.0f, 2.0f);
	LensSettings.DiaphragmBladeCount = FMath::Clamp(DiaphragmBladeCount, 4, 16);
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		Cine->SetLensSettings(LensSettings);
	}
	FOV = GetSensorFOV();
}

void UFusionCamSensor::SetCineFocusDistance(float FocusDistance)
{
	SetCineCameraEnabled(true);
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		FCameraFocusSettings Settings = Cine->FocusSettings;
		Settings.FocusMethod = ECameraFocusMethod::Manual;
		Settings.ManualFocusDistance = FMath::Max(FocusDistance, 0.0f);
		Cine->SetFocusSettings(Settings);
	}
}

bool UFusionCamSensor::SetCineFocusMode(const FString& FocusMode, bool bSmoothFocusChanges, float SmoothingSpeed, float FocusOffset)
{
	ECameraFocusMethod Method;
	if (FocusMode.Equals(TEXT("manual"), ESearchCase::IgnoreCase)) Method = ECameraFocusMethod::Manual;
	else if (FocusMode.Equals(TEXT("tracking"), ESearchCase::IgnoreCase)) Method = ECameraFocusMethod::Tracking;
	else if (FocusMode.Equals(TEXT("disable"), ESearchCase::IgnoreCase)) Method = ECameraFocusMethod::Disable;
	else if (FocusMode.Equals(TEXT("none"), ESearchCase::IgnoreCase) ||
		FocusMode.Equals(TEXT("do_not_override"), ESearchCase::IgnoreCase)) Method = ECameraFocusMethod::DoNotOverride;
	else return false;

	SetCineCameraEnabled(true);
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		FCameraFocusSettings Settings = Cine->FocusSettings;
		Settings.FocusMethod = Method;
		Settings.bSmoothFocusChanges = bSmoothFocusChanges;
		Settings.FocusSmoothingInterpSpeed = FMath::Max(SmoothingSpeed, 0.0f);
		Settings.FocusOffset = FocusOffset;
		Cine->SetFocusSettings(Settings);
	}
	return true;
}

bool UFusionCamSensor::SetCineTrackingFocus(AActor* ActorToTrack, const FVector& RelativeOffset)
{
	if (!IsValid(ActorToTrack)) return false;
	SetCineCameraEnabled(true);
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		FCameraFocusSettings Settings = Cine->FocusSettings;
		Settings.FocusMethod = ECameraFocusMethod::Tracking;
		Settings.TrackingFocusSettings.ActorToTrack = ActorToTrack;
		Settings.TrackingFocusSettings.RelativeOffset = RelativeOffset;
		Cine->SetFocusSettings(Settings);
	}
	return true;
}

void UFusionCamSensor::SetCineCrop(float CropAspectRatio, float Overscan, bool bCropOverscan, bool bScaleResolutionWithOverscan)
{
	SetCineCameraEnabled(true);
	FPlateCropSettings CropSettings;
	CropSettings.AspectRatio = FMath::Max(CropAspectRatio, 0.0f);
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		Cine->SetCropSettings(CropSettings);
		Cine->SetOverscan(FMath::Max(Overscan, 0.0f));
		Cine->SetCropOverscan(bCropOverscan);
		Cine->SetScaleResolutionWithOverscan(bScaleResolutionWithOverscan);
	}
	FOV = GetSensorFOV();
}

void UFusionCamSensor::SetCineNearClip(bool bEnabled, float NearClipDistance)
{
	SetCineCameraEnabled(true);
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		Cine->bOverride_CustomNearClippingPlane = bEnabled;
		Cine->SetCustomNearClippingPlane(FMath::Max(NearClipDistance, 0.00001f));
	}
}

void UFusionCamSensor::SetCineExposure(float ISO, float ShutterSpeed, bool bApplyPhysicalExposure)
{
	SetCineCameraEnabled(true);
	const float SafeISO = FMath::Max(ISO, 1.0f);
	const float SafeShutterSpeed = FMath::Max(ShutterSpeed, 0.001f);
	for (UCineCameraComponent* Cine : GetCineCameraComponents())
	{
		Cine->ExposureMethod = bApplyPhysicalExposure ? ECameraExposureMethod::Enabled : ECameraExposureMethod::DoNotOverride;
		Cine->PostProcessSettings.bOverride_CameraISO = true;
		Cine->PostProcessSettings.CameraISO = SafeISO;
		Cine->PostProcessSettings.bOverride_CameraShutterSpeed = true;
		Cine->PostProcessSettings.CameraShutterSpeed = SafeShutterSpeed;
		Cine->PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
		Cine->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = bApplyPhysicalExposure;
	}
	for (UBaseCameraSensor* Sensor : FusionSensors)
	{
		if (!IsValid(Sensor)) continue;
		Sensor->PostProcessSettings.bOverride_CameraISO = true;
		Sensor->PostProcessSettings.CameraISO = SafeISO;
		Sensor->PostProcessSettings.bOverride_CameraShutterSpeed = true;
		Sensor->PostProcessSettings.CameraShutterSpeed = SafeShutterSpeed;
		Sensor->PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
		Sensor->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = bApplyPhysicalExposure;
		Sensor->PostProcessBlendWeight = 1.0f;
		Sensor->UpdateCineCameraView();
	}
}

TArray<UFusionCamSensor*> UFusionCamSensor::GetComponents(AActor* Actor)
{
	TArray<UFusionCamSensor*> Components;
	if (!IsValid(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Actor is invalid"));
		return Components;
	}

	TArray<UActorComponent*> ChildComponents = Actor->K2_GetComponentsByClass(UFusionCamSensor::StaticClass());
	for (UActorComponent* Component : ChildComponents)
	{
		Components.Add(Cast<UFusionCamSensor>(Component));
	}
	return Components;
}

#if WITH_EDITOR
void UFusionCamSensor::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UFusionCamSensor, PresetFilmSize))
	{
		switch(PresetFilmSize)
		{
		case EPresetFilmSize::F640x480:
			SetFilmSize(640, 480);
			break;
		case EPresetFilmSize::F1080p:
			SetFilmSize(1920, 1080);
			break;
		case EPresetFilmSize::F720p:
			SetFilmSize(1280, 720);
			break;
		}
	}
}
#endif


void UFusionCamSensor::SetProjectionType(ECameraProjectionMode::Type ProjectionType)
{
	for (int i = 0; i < FusionSensors.Num(); i++)
	{
		UBaseCameraSensor* Sensor = FusionSensors[i];
		if (IsValid(Sensor))
		{
			Sensor->ProjectionType = ProjectionType;
		}
		else
		{
			// UE_LOG(LogTemp, Error, TEXT("SetProjectionType: Sensor %d within FusionCamSensor is invalid."), i);
			UE_LOG(LogTemp, Warning, TEXT("SetFilmSize: Sensor %d within FusionCamSensor is invalid."), i);
		}
	}
}

void UFusionCamSensor::SetOrthoWidth(float OrthoWidth)
{
	for (int i = 0; i < FusionSensors.Num(); i++)
	{
		UBaseCameraSensor* Sensor = FusionSensors[i];
		if (!IsValid(Sensor))
		{
			// UE_LOG(LogTemp, Error, TEXT("SetOrthoWidth: Sensor %d within FusionCamSensor is invalid."), i);
			UE_LOG(LogTemp, Warning, TEXT("SetFilmSize: Sensor %d within FusionCamSensor is invalid."), i);
			continue;
		}
		Sensor->OrthoWidth = OrthoWidth;
	}
}

void UFusionCamSensor::SetLitCaptureSource(ESceneCaptureSource CaptureSource)
{
    this->LitCamSensor->CaptureSource = CaptureSource;
}

// Configure the post process settings
void UFusionCamSensor::SetReflectionMethod(EReflectionMethod::Type Method)
{
    // None, Lumen, ScreenSpace, RayTraced
    this->LitCamSensor->PostProcessSettings.bOverride_ReflectionMethod = true;
    this->LitCamSensor->PostProcessSettings.ReflectionMethod = Method;
}

void UFusionCamSensor::SetGlobalIlluminationMethod(EDynamicGlobalIlluminationMethod::Type Method)
{
    // None, Lumen, ScreenSpace, RayTraced, Plugin,
    this->LitCamSensor->PostProcessSettings.bOverride_DynamicGlobalIlluminationMethod = true;
    this->LitCamSensor->PostProcessSettings.DynamicGlobalIlluminationMethod = Method;
}

void UFusionCamSensor::SetExposureMethod(EAutoExposureMethod Method)
{
    this->LitCamSensor->PostProcessSettings.bOverride_AutoExposureMethod = true;
    this->LitCamSensor->PostProcessSettings.AutoExposureMethod = Method;
}

void UFusionCamSensor::SetExposureBias(float ExposureBias)
{
    this->LitCamSensor->PostProcessSettings.bOverride_AutoExposureBias = true;
    this->LitCamSensor->PostProcessSettings.AutoExposureBias = ExposureBias;
}

void UFusionCamSensor::SetAutoExposureSpeed(float SpeedDown, float SpeedUp)
{
    this->LitCamSensor->PostProcessSettings.bOverride_AutoExposureSpeedDown = true;
    this->LitCamSensor->PostProcessSettings.AutoExposureSpeedDown = SpeedDown;
    this->LitCamSensor->PostProcessSettings.bOverride_AutoExposureSpeedUp = true;
    this->LitCamSensor->PostProcessSettings.AutoExposureSpeedUp = SpeedUp;
}

void UFusionCamSensor::SetAutoExposureBrightness(float MinBrightness, float MaxBrightness)
{
    // Brightness range for the auto exposure algorithm
    if (MinBrightness > MaxBrightness)
    {
        UE_LOG(LogUnrealCV, Warning, TEXT("MinBrightness should be smaller than MaxBrightness"));
        return;
    }
    // Auto-Exposure minimum adaptation.
    this->LitCamSensor->PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
    this->LitCamSensor->PostProcessSettings.AutoExposureMinBrightness = MinBrightness;
    // Auto-Exposure
    this->LitCamSensor->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
    this->LitCamSensor->PostProcessSettings.AutoExposureMaxBrightness = MaxBrightness;
}

void UFusionCamSensor::SetApplyPhysicalCameraExposure(int ApplyPhysicalCameraExposure)
{
    this->LitCamSensor->PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
    this->LitCamSensor->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = ApplyPhysicalCameraExposure;
}


void UFusionCamSensor::SetMotionBlurParams(float MotionBlurAmount, float MotionBlurMax, float MotionBlurPerObjectSize, int MotionBlurTargetFPS)
{
    // Strength of motion blur, 0:off
    this->LitCamSensor->PostProcessSettings.bOverride_MotionBlurAmount = true;
    this->LitCamSensor->PostProcessSettings.MotionBlurAmount = MotionBlurAmount;
    // Max distortion caused by motion blur, in percent of the screen width, 0:off
    this->LitCamSensor->PostProcessSettings.bOverride_MotionBlurMax = true;
    this->LitCamSensor->PostProcessSettings.MotionBlurMax = MotionBlurMax;
    // The minimum projected screen radius for a primitive to be drawn in the velocity pass, percentage of screen width.
    this->LitCamSensor->PostProcessSettings.bOverride_MotionBlurPerObjectSize = true;
    this->LitCamSensor->PostProcessSettings.MotionBlurPerObjectSize = MotionBlurPerObjectSize;
    // Target frame rate for motion blur
    this->LitCamSensor->PostProcessSettings.bOverride_MotionBlurTargetFPS = true;
    this->LitCamSensor->PostProcessSettings.MotionBlurTargetFPS = MotionBlurTargetFPS;
}

void UFusionCamSensor::SetFocalParams(float FocalDistance, float FocalRegion)
{
    this->LitCamSensor->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
    this->LitCamSensor->PostProcessSettings.DepthOfFieldFocalDistance = FocalDistance;
    this->LitCamSensor->PostProcessSettings.bOverride_DepthOfFieldFocalRegion = true;
    this->LitCamSensor->PostProcessSettings.DepthOfFieldFocalRegion = FocalRegion;
}
