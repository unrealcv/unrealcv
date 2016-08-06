#include "UnrealCVPrivate.h"
#include "GTCapturer.h"
#include "ImageWrapper.h"

void InitCaptureComponent(USceneCaptureComponent2D* CaptureComponent)
{
	// Can not use ESceneCaptureSource::SCS_SceneColorHDR, this option will disable post-processing 
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	CaptureComponent->TextureTarget = NewObject<UTextureRenderTarget2D>();
	CaptureComponent->TextureTarget->InitAutoFormat(640, 480); // TODO: Update this later
	// CaptureComponent->TextureTarget->TargetGamma = 2.2;
	// CaptureComponent->TextureTarget->TargetGamma = 1 / 2.2;
	// CaptureComponent->TextureTarget->TargetGamma = GEngine->GetDisplayGamma();
	// CaptureComponent->TextureTarget->TargetGamma = 1;
	// CaptureComponent->TextureTarget->TargetGamma = 2.2;
	CaptureComponent->TextureTarget->TargetGamma = 1;
	
	CaptureComponent->RegisterComponentWithWorld(GWorld); // What happened for this?
	// CaptureComponent->AddToRoot(); This is not necessary since it has been attached to the Pawn.
}


void SaveExr(UTextureRenderTarget2D* RenderTarget, FString Filename)
{
	int32 Width = RenderTarget->SizeX, Height = RenderTarget->SizeY;
	TArray<FFloat16Color> FloatImage;
	FloatImage.AddZeroed(Width * Height);
	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	RenderTargetResource->ReadFloat16Pixels(FloatImage);

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::EXR);

	ImageWrapper->SetRaw(FloatImage.GetData(), FloatImage.GetAllocatedSize(), Width, Height, ERGBFormat::RGBA, 16);
	const TArray<uint8>& PngData = ImageWrapper->GetCompressed(ImageCompression::Uncompressed);
	FFileHelper::SaveArrayToFile(PngData, *Filename);
}

void SavePng(UTextureRenderTarget2D* RenderTarget, FString Filename)
{
	int32 Width = RenderTarget->SizeX, Height = RenderTarget->SizeY;
	TArray<FColor> Image;
	Image.AddZeroed(Width * Height);
	FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	RenderTargetResource->ReadPixels(Image);

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	ImageWrapper->SetRaw(Image.GetData(), Image.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, 8);
	const TArray<uint8>& HDRData = ImageWrapper->GetCompressed(ImageCompression::Uncompressed);
	FFileHelper::SaveArrayToFile(HDRData, *Filename);
}

UMaterial* LoadMaterial(FString InModeName = TEXT(""))
{
	// Load material for visualization
	static TMap<FString, FString>* MaterialPathMap = nullptr;
	if (MaterialPathMap == nullptr)
	{ 
		// MaterialPathMap = NewObject<TMap<FString, FString> >();
		MaterialPathMap = new TMap<FString, FString>();
		// MaterialPathMap->Add(TEXT("depth"), TEXT("Material'/UnrealCV/SceneDepth.SceneDepth'"));
		MaterialPathMap->Add(TEXT("debug"), TEXT("Material'/UnrealCV/debug.debug'"));
		MaterialPathMap->Add(TEXT("depth"), TEXT("Material'/UnrealCV/debug.debug'"));
		// MaterialPathMap->Add(TEXT("depth"), TEXT("Material'/Game/SceneDepth.SceneDepth'"));
	}

	static TMap<FString, UMaterial*>* StaticMaterialMap = nullptr;
	if (StaticMaterialMap == nullptr)
	{
		StaticMaterialMap = new TMap<FString, UMaterial*>();
		for (auto& Elem : *MaterialPathMap)
		{
			FString ModeName = Elem.Key;
			FString MaterialPath = Elem.Value;
			static ConstructorHelpers::FObjectFinder<UMaterial> Material(*MaterialPath); // ConsturctorHelpers is only available for UObject.

			if (Material.Object != NULL)
			{
				StaticMaterialMap->Add(ModeName, (UMaterial*)Material.Object);
			}
		}
	}

	UMaterial* Material = StaticMaterialMap->FindRef(InModeName);
	if (Material == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can not recognize visualization mode %s"), *InModeName);
	}
	return Material;
}

UGTCapturer* UGTCapturer::Create(APawn* InPawn, FString Mode)
{
	UGTCapturer* GTCapturer = NewObject<UGTCapturer>();

	GTCapturer->Pawn = InPawn;
	GTCapturer->AddToRoot();

	// DEPRECATED_FORGAME(4.6, "CaptureComponent2D should not be accessed directly, please use GetCaptureComponent2D() function instead. CaptureComponent2D will soon be private and your code will not compile.")
	USceneCaptureComponent2D* CaptureComponent = NewObject<USceneCaptureComponent2D>(); 
	GTCapturer->CaptureComponent = CaptureComponent;
		
	// CaptureComponent needs to be attached to somewhere immediately, otherwise it will be gc-ed
	CaptureComponent->AttachTo(InPawn->GetRootComponent());
	InitCaptureComponent(CaptureComponent);

	UMaterial* Material = LoadMaterial(Mode);
	if (Mode == "lit") // For rendered images
	{
		FEngineShowFlags& ShowFlags = CaptureComponent->ShowFlags;

		ApplyViewMode(VMI_Lit, true, ShowFlags);
		ShowFlags.SetMaterials(true);
		ShowFlags.SetLighting(true);
		ShowFlags.SetPostProcessing(true);
		ShowFlags.SetTonemapper(true);

		// ToneMapper needs to be enabled, or the screen will be very dark
		// TemporalAA needs to be disabled, otherwise the previous frame might contaminate current frame.
		// Check: https://answers.unrealengine.com/questions/436060/low-quality-screenshot-after-setting-the-actor-pos.html for detail
		// Viewport->EngineShowFlags.SetTemporalAA(false);
		ShowFlags.SetTemporalAA(true);
	}
	else if (Mode == "object_mask") // For object mask
	{ 
		FEngineShowFlags& ShowFlags = CaptureComponent->ShowFlags;
		ShowFlags.SetVertexColors(true);
		ShowFlags.SetTonemapper(false);

		ShowFlags.SetMaterials(false);
		ShowFlags.SetLighting(false);
		ShowFlags.SetBSPTriangles(true);
		ShowFlags.SetVertexColors(true);
		ShowFlags.SetPostProcessing(false);
		ShowFlags.SetHMDDistortion(false);
		ShowFlags.SetTonemapper(false); // This won't take effect here

		GVertexColorViewMode = EVertexColorViewMode::Color;
	}
	else 
	{
		check(Material);
		CaptureComponent->PostProcessSettings.AddBlendable(Material, 1);
		CaptureComponent->ShowFlags.SetTonemapper(false);
	}
	return GTCapturer;
}

UGTCapturer::UGTCapturer()
{
	LoadMaterial();
	// bIsTicking = false;

	// Create USceneCaptureComponent2D
	// Design Choice: do I need one capture component with changing post-process materials or multiple components?
	// USceneCaptureComponent2D* CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("GTCaptureComponent"));
}

// Each GTCapturer can serve as one camera of the scene

bool UGTCapturer::Capture(FString InFilename)
{
	if (!bIsPending) // TODO: Use a filename queue to replace this flag.
	{
		FlushRenderingCommands();
		bIsPending = true;
		this->Filename = InFilename;
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("A capture command is still pending."));
		return false;
	}
	// TODO: only enable USceneComponentCapture2D's rendering flag, when I require it to do so.
}

void UGTCapturer::Tick(float DeltaTime) // This tick function should be called by the scene instead of been 
{
	// Update rotation of each frame
	// from ab237f46dc0eee40263acbacbe938312eb0dffbb:CameraComponent.cpp:232
	const APawn* OwningPawn = this->Pawn;
	const AController* OwningController = OwningPawn ? OwningPawn->GetController() : nullptr;
	if (OwningController && OwningController->IsLocalPlayerController())
	{
		const FRotator PawnViewRotation = OwningPawn->GetViewRotation();
		if (!PawnViewRotation.Equals(CaptureComponent->GetComponentRotation()))
		{
			CaptureComponent->SetWorldRotation(PawnViewRotation);
		}
	}

	if (bIsPending)
	{
		FString LowerCaseFilename = this->Filename.ToLower();
		if (LowerCaseFilename.EndsWith("png"))
		{
			SavePng(CaptureComponent->TextureTarget, this->Filename);
		}
		else if (LowerCaseFilename.EndsWith("exr"))
		{
			SaveExr(CaptureComponent->TextureTarget, this->Filename);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Unrecognized image file extension %s"), *LowerCaseFilename);
		}
		// TODO: Run a callback when these operations finished.

		bIsPending = false; // Only tick once.
	}
}