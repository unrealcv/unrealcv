// Weichao Qiu @ 2017
#include "BaseCameraSensor.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Engine/CollisionProfile.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "TextureReader.h"
#include "UnrealcvServer.h"
#include "UnrealcvStats.h"
#include "UnrealcvLog.h"
#include "ImageUtil.h"

DECLARE_CYCLE_STAT(TEXT("ReadBuffer"), STAT_ReadBuffer, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("ReadBufferFast"), STAT_ReadBufferFast, STATGROUP_UnrealCV);
// DECLARE_CYCLE_STAT(TEXT("ReadPixels"), STAT_ReadPixels, STATGROUP_UnrealCV);

// FImageWorker UBaseCameraSensor::ImageWorker;

UBaseCameraSensor::UBaseCameraSensor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// static ConstructorHelpers::FObjectFinder<UStaticMesh> EditorCameraMesh(TEXT("/Engine/EditorMeshes/MatineeCam_SM"));
	// Another choice is "StaticMesh'/Engine/EditorMeshes/Camera/SM_CineCam.SM_CineCam'"
	this->ShowFlags.SetPostProcessing(true);
	bCaptureEveryFrame = false;
	bCaptureOnMovement = false;
	CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	bAlwaysPersistRenderingState = true; 

	FServerConfig& Config = FUnrealcvServer::Get().Config;
	FilmWidth = Config.Width == 0 ? 640 : Config.Width;
	FilmHeight = Config.Height == 0 ? 480 : Config.Height;
	FOVAngle = Config.FOV == 0 ? 90 : Config.FOV; 
	// Avoid calling virtual function in a constructor
}

// Explicitly make a request to render frames
// This is needed if we want to disable bCaptureEveryFrame
// https://answers.unrealengine.com/questions/723947/scene-capture-with-post-process-mat-works-only-wit.html?sort=oldest

// if (GetOwner()) // Check whether this is a template project
// if (!IsTemplate())

// NOTE: Avoid creating TextureTarget in the CTOR, this will make CamSensor not savable in a BP actor
// TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CamSensorRenderTarget"));

void UBaseCameraSensor::InitTextureTarget(int FilmWidth, int FilmHeight)
{
	// bool bUseLinearGamma = false;
	EPixelFormat PixelFormat = EPixelFormat::PF_B8G8R8A8;
	bool bUseLinearGamma = false;
	TextureTarget = NewObject<UTextureRenderTarget2D>(this); 
	TextureTarget->InitCustomFormat(FilmWidth, FilmHeight, PixelFormat, bUseLinearGamma);
}

void UBaseCameraSensor::SetFilmSize(int Width, int Height)
{
	this->FilmWidth = Width;
	this->FilmHeight = Height;
	if (!IsValid(TextureTarget))
	{
		TextureTarget = NewObject<UTextureRenderTarget2D>(this); 
		// TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("CamSensorRenderTarget"));
	}

	if (TextureTarget->SizeX != Width || TextureTarget->SizeY != Height) 
	{
		InitTextureTarget(Width, Height);
	}
}

int UBaseCameraSensor::GetFilmWidth()
{
	if (!IsValid(TextureTarget)) return 0;
	return TextureTarget->SizeX;
}

int UBaseCameraSensor::GetFilmHeight()
{
	if (!IsValid(TextureTarget)) return 0;
	return TextureTarget->SizeY;
}


// TODO: Split the logic, move data serialization code outside
// Serialize the data to png and npy, check the speed.

// EPixelFormat::PF_B8G8R8A8
/*
This is defined in FColor
	#ifdef _MSC_VER
	// Win32 x86
	union { struct{ uint8 B,G,R,A; }; uint32 AlignmentDummy; };
#else
	// Linux x86, etc
	uint8 B GCC_ALIGN(4);
	uint8 G,R,A;
*/
bool UBaseCameraSensor::CheckTextureTarget()
{
	if (!IsValid(TextureTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("The TextureTarget was not initialized."));
		return false;
	}
	if (TextureTarget->SizeX == 0 || TextureTarget->SizeY == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("The TextureTarget has invalid size."));
		return false;
	}
	return true;
}

void UBaseCameraSensor::CaptureFast(TArray<FColor>& ImageData, int& Width, int& Height)
{
	if (!CheckTextureTarget()) return;
	this->CaptureScene();

	if (TextureTarget == nullptr)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("TextureTarget has not been initialized"));
		return;
	}
	FTextureRenderTargetResource* RenderTargetResource = this->TextureTarget->GameThread_GetRenderTargetResource();
	if (RenderTargetResource == nullptr)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("RenderTargetResource is nullptr"));
		return;
	}
	FTexture2DRHIRef Texture2D = RenderTargetResource->GetRenderTargetTexture();

	static TFunction<void(FColor*, int32, int32)> Callback = [&](FColor* ColorPtr, int InWidth, int InHeight)
	{
		// Copy data to ImageData
		Width = InWidth;
		Height = InHeight;

		ImageData.AddZeroed(Width * Height);
		FColor* DestPtr = ImageData.GetData();
		for (int32 Row = 0; Row < Height; ++Row)
		{
			FMemory::Memcpy(DestPtr, ColorPtr, sizeof(FColor)*Width);
			ColorPtr += Width;
			DestPtr += Width;
		}
	};

	FastReadTexture2DAsync(Texture2D, Callback);
	FlushRenderingCommands(); // Ensure the callback is done
}


void UBaseCameraSensor::CaptureToFile(const FString& Filename)
{
	if (!CheckTextureTarget()) return;
	this->CaptureScene();
	
	FTextureRenderTargetResource* RenderTargetResource = this->TextureTarget->GameThread_GetRenderTargetResource();
	FTexture2DRHIRef Texture2D = RenderTargetResource->GetRenderTargetTexture();

	/** Provide funtions to convert image format and save file */
	static auto Callback = [=](FColor* ColorBuffer, int Width, int Height)
	{
		// Initialize the image data array
		TArray<FColor> Dest; // Write this.
		Dest.AddZeroed(Width * Height); // TODO: will break!
		FColor* DestPtr = Dest.GetData();

		for (int32 Row = 0; Row < Height; ++Row)
		{
			FMemory::Memcpy(DestPtr, ColorBuffer, sizeof(FColor)*Width);
			ColorBuffer += Width;
			DestPtr += Width;
		}

		// ImageWorker.SaveFile(Dest, Width, Height, Filename);
		// ImageUtil.SaveBmpFile(Dest, Width, Height, FString::Printf(TEXT("%s_%d.bmp"), *Filename, GFrameNumber));
		FImageUtil ImageUtil;
		ImageUtil.SaveBmpFile(Dest, Width, Height, Filename);
	};

	FastReadTexture2DAsync(Texture2D, Callback);
}

void UBaseCameraSensor::Capture(TArray<FColor>& ImageData, int& Width, int& Height)
{
	SCOPE_CYCLE_COUNTER(STAT_ReadBuffer);

	if (!CheckTextureTarget()) return;
	this->CaptureScene();

	UTextureRenderTarget2D* RenderTarget = this->TextureTarget;
	ReadTextureRenderTarget(RenderTarget, ImageData, Width, Height);
}

void UBaseCameraSensor::SetPostProcessMaterial(UMaterial* PostProcessMaterial)
{
	PostProcessSettings.AddBlendable(PostProcessMaterial, 1);
}

void UBaseCameraSensor::GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView)
{
	DesiredView.Location = GetComponentLocation();
	DesiredView.Rotation = GetComponentRotation();
	DesiredView.FOV = this->FOVAngle;
	// DesiredView.FOV = FieldOfView;
	// DesiredView.bConstrainAspectRatio = bConstrainAspectRatio;
	// DesiredView.bUseFieldOfViewForLOD = bUseFieldOfViewForLOD;
	// DesiredView.ProjectionMode = ProjectionMode;
	DesiredView.ProjectionMode = ECameraProjectionMode::Perspective;
	DesiredView.OrthoWidth = OrthoWidth;
	// DesiredView.OrthoNearClipPlane = OrthoNearClipPlane;
	// DesiredView.OrthoFarClipPlane = OrthoFarClipPlane;

	// See if the CameraActor wants to override the PostProcess settings used.
	DesiredView.PostProcessBlendWeight = PostProcessBlendWeight;
	if (PostProcessBlendWeight > 0.0f)
	{
		DesiredView.PostProcessSettings = PostProcessSettings;
	}

}
