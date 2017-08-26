// Fill out your copyright notice in the Description page of Project Settings.
#include "UnrealCVPrivate.h"
#include "UnrealCV.h"
#include "ImageSaver.h"
#include "UE4CVServer.h"
#include "CvCameraComponent.h"

UCvCameraComponent::UCvCameraComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// TODO: Read this from the configuration file.
	// Width = 640; Height = 480;
	FServerConfig Config = FUE4CVServer::Get().Config;
	ImageWidth = Config.Width; ImageHeight = Config.Height;

	DepthCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("DepthCaptureComponent"));
	// DepthCaptureComponent->SetupAttachment(this);
	DepthCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	DepthCaptureComponent->bCaptureEveryFrame = false;

	/*
	NormalCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("NormalCaptureComponent"));
	NormalCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_Normal;
	NormalCaptureComponent->TextureTarget = NewObject<UTextureRenderTarget2D>();
	NormalCaptureComponent->TextureTarget->InitAutoFormat(Width, Height);
	*/

	LitCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("LitCaptureComponent"));
	LitCaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
	LitCaptureComponent->bCaptureEveryFrame = false;
	// LitCaptureComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform); // Need to attach it, otherwise it will be at (0, 0, 0).
	if (! this->IsTemplate())
	{
		DepthCaptureComponent->SetupAttachment(this);
		LitCaptureComponent->SetupAttachment(this);
	}
}

UCvCameraComponent::UCvCameraComponent()
{
}

UCvCameraComponent::~UCvCameraComponent()
{
}

void UCvCameraComponent::BeginPlay()
{
	// bool bTrasient = true;
	// LitCaptureComponent->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("LitRenderTarget2D"), bTrasient);
	if (LitCaptureComponent->TextureTarget == nullptr)
	{
		LitCaptureComponent->TextureTarget = NewObject<UTextureRenderTarget2D>(); // This will fail serialization
		LitCaptureComponent->TextureTarget->InitAutoFormat(ImageWidth, ImageHeight);
		LitCaptureComponent->TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
	}

	if (DepthCaptureComponent->TextureTarget == nullptr)
	{
		// DepthCaptureComponent->TextureTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(TEXT("DepthRenderTarget2D"));
		DepthCaptureComponent->TextureTarget = NewObject<UTextureRenderTarget2D>();
		DepthCaptureComponent->TextureTarget->InitAutoFormat(ImageWidth, ImageHeight);
	}
}

void UCvCameraComponent::CaptureDepth(TArray<FFloat16Color>& FloatImage, int& Width, int& Height)
{
    ScreenLog("Capture depth");
	ScreenLog(this->GetName());
	this->DepthCaptureComponent->CaptureScene();
	FTextureRenderTargetResource *RenderTargetResource = this->DepthCaptureComponent->TextureTarget->GameThread_GetRenderTargetResource();

	Width = this->DepthCaptureComponent->TextureTarget->GetSurfaceWidth();
	Height = this->DepthCaptureComponent->TextureTarget->GetSurfaceHeight();
	RenderTargetResource->ReadFloat16Pixels(FloatImage);
}

void UCvCameraComponent::CaptureDepthToFile(FString Filename)
{
	TArray<FFloat16Color> FloatImage;
	int Width = 0, Height = 0;
	CaptureDepth(FloatImage, Width, Height);
	FImageSaver::SaveFile(FloatImage, Width, Height, Filename);
}

void UCvCameraComponent::CaptureNormal()
{
	/*
    ScreenLog("Capture normal");
	this->NormalCaptureComponent->CaptureScene();
	FTextureRenderTargetResource *RenderTargetResource = this->NormalCaptureComponent->TextureTarget->GameThread_GetRenderTargetResource();

	TArray<FColor> Image;
	RenderTargetResource->ReadPixels(Image);
	*/
}


void UCvCameraComponent::CaptureImageToFile(FString Filename)
{
	TArray<FColor> Image;
	int Width = 0, Height = 0;
	CaptureImage(Image, Width, Height);
	FImageSaver::SaveFile(Image, Width, Height, Filename);
}

void UCvCameraComponent::CaptureImage(TArray<FColor>& Image, int& Width, int& Height)
{
	ScreenLog("Capture image");
	{
		SCOPE_CYCLE_COUNTER(STAT_CaptureScene);
		this->LitCaptureComponent->CaptureScene();
	}
	FTextureRenderTargetResource *RenderTargetResource = this->LitCaptureComponent->TextureTarget->GameThread_GetRenderTargetResource();
	{
		SCOPE_CYCLE_COUNTER(STAT_ReadPixels);
		// static TArray<FColor> *Image = new TArray<FColor>();
		Width = this->LitCaptureComponent->TextureTarget->GetSurfaceWidth();
		Height = this->LitCaptureComponent->TextureTarget->GetSurfaceHeight();

		FReadSurfaceDataFlags ReadSurfaceDataFlags;
		ReadSurfaceDataFlags.SetLinearToGamma(false); // This is super important to disable this!
		RenderTargetResource->ReadPixels(Image, ReadSurfaceDataFlags); // TODO: This is very slow.
		// ReadPixels(RenderTargetResource, *Image);
	}
}

// Write a faster version for this function!

// bool ReadPixels(FTextureRenderTargetResource *RenderTargetResource, TArray< FColor >& OutImageData,
// 	//FReadSurfaceDataFlags InFlags, FIntRect InRect
// 	FReadSurfaceDataFlags InFlags = FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX), FIntRect InRect = FIntRect(0, 0, 0, 0))
// {
// 	if (InRect == FIntRect(0, 0, 0, 0))
// 	{
// 		InRect = FIntRect(0, 0, RenderTargetResource->GetSizeXY().X, RenderTargetResource->GetSizeXY().Y);
// 	}
//
// 	// Read the render target surface data back.
// 	struct FReadSurfaceContext
// 	{
// 		FRenderTarget* SrcRenderTarget;
// 		TArray<FColor>* OutData;
// 		FIntRect Rect;
// 		FReadSurfaceDataFlags Flags;
// 	};
//
// 	OutImageData.Reset();
// 	FReadSurfaceContext ReadSurfaceContext =
// 	{
// 		RenderTargetResource,
// 		&OutImageData,
// 		InRect,
// 		InFlags
// 	};
//
// 	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
// 		ReadSurfaceCommand,
// 		FReadSurfaceContext, Context, ReadSurfaceContext,
// 		{
// 			SCOPE_CYCLE_COUNTER(STAT_ReadSurfaceData);
// 			RHICmdList.ReadSurfaceData(
// 				Context.SrcRenderTarget->GetRenderTargetTexture(),
// 				Context.Rect,
// 				*Context.OutData,
// 				Context.Flags
// 			);
// 		});
// 	// FlushRenderingCommands();
//
// 	return OutImageData.Num() > 0;
// }
