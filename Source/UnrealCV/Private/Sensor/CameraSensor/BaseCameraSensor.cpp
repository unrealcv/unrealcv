// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "BaseCameraSensor.h"
// #include "GlobalShader.h"
// #include "Shader.h"
// #include "ScreenRendering.h"
// #include "SceneRendering.h"
// #include "ScenePrivate.h"
// #include "ClearQuad.h"
// #include "JsonObjectConverter.h"
#include "TextureReader.h"
#include "FileHelper.h"

DECLARE_CYCLE_STAT(TEXT("ReadBuffer"), STAT_ReadBuffer, STATGROUP_UnrealCV);
DECLARE_CYCLE_STAT(TEXT("ReadBufferFast"), STAT_ReadBufferFast, STATGROUP_UnrealCV);
// DECLARE_CYCLE_STAT(TEXT("ReadPixels"), STAT_ReadPixels, STATGROUP_UnrealCV);

FImageWorker UBaseCameraSensor::ImageWorker;

UBaseCameraSensor::UBaseCameraSensor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> EditorCameraMesh(TEXT("/Engine/EditorMeshes/MatineeCam_SM"));
	// static ConstructorHelpers::FObjectFinder<UStaticMesh> EditorCameraMesh(TEXT("StaticMesh'/Engine/EditorMeshes/Camera/SM_CineCam.SM_CineCam'"));
	CameraMesh = EditorCameraMesh.Object;

	this->ShowFlags.SetPostProcessing(true);
	
	FServerConfig& Config = FUE4CVServer::Get().Config;
	FilmWidth = Config.Width;
	FilmHeight = Config.Height; 
	this->FOVAngle = Config.FOV;
}

void UBaseCameraSensor::OnRegister()
{
	Super::OnRegister();

	// Explicitly make a request to render frames
	this->bCaptureEveryFrame = false;
	this->bCaptureOnMovement = false;

	// Add a visualization camera mesh
	if (GetOwner()) // Check whether this is a template project
	// if (!IsTemplate())
	{
		ProxyMeshComponent = NewObject<UStaticMeshComponent>(GetOwner(), NAME_None, RF_Transactional | RF_TextExportTransient);
		// ProxyMeshComponent = this->CreateDefaultSubobject<UStaticMeshComponent>("Visualization Camera Mesh");
		ProxyMeshComponent->SetupAttachment(this);
		ProxyMeshComponent->bIsEditorOnly = true;
		ProxyMeshComponent->SetStaticMesh(CameraMesh);
		ProxyMeshComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		ProxyMeshComponent->bHiddenInGame = true;
		ProxyMeshComponent->CastShadow = false;
		ProxyMeshComponent->PostPhysicsComponentTick.bCanEverTick = false;
		ProxyMeshComponent->CreationMethod = CreationMethod;
		ProxyMeshComponent->RegisterComponentWithWorld(GetWorld());
	}

}


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

void UBaseCameraSensor::CaptureFast(TArray<FColor>& ImageData, int& Width, int& Height)
{
	TFunction<void(FColor*, int32, int32)> Callback = [&](FColor* ColorPtr, int InWidth, int InHeight)
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
	FastReadTexture2DAsync(Texture2D, Callback);
	FlushRenderingCommands(); // Ensure the callback is done
}


void UBaseCameraSensor::CaptureToFile(const FString& Filename)
{
	auto Callback = [=](FColor* ColorBuffer, int Width, int Height)
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
		ImageUtil.SaveBmpFile(Dest, Width, Height, Filename);
	};
	this->CaptureScene();
	check(this->TextureTarget);
	FTextureRenderTargetResource* RenderTargetResource = this->TextureTarget->GameThread_GetRenderTargetResource();
	FTexture2DRHIRef Texture2D = RenderTargetResource->GetRenderTargetTexture();
	FastReadTexture2DAsync(Texture2D, Callback);
}

void UBaseCameraSensor::Capture(TArray<FColor>& ImageData, int& Width, int& Height)
{
	SCOPE_CYCLE_COUNTER(STAT_ReadBuffer);

	this->CaptureScene();
	check(this->TextureTarget);
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
