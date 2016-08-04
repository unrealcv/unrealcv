#include "UnrealCVPrivate.h"
#include "GTCapturer.h"
#include "ImageWrapper.h"

void InitCaptureComponent(USceneCaptureComponent2D* CaptureComponent)
{
	// Can not use ESceneCaptureSource::SCS_SceneColorHDR, this option will disable post-processing 
	CaptureComponent->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

	CaptureComponent->TextureTarget = NewObject<UTextureRenderTarget2D>();
	CaptureComponent->TextureTarget->InitAutoFormat(640, 480); // TODO: Update this later
	
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
		MaterialPathMap->Add(TEXT("depth"), TEXT("Material'/Game/SceneDepth.SceneDepth'"));
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
	if (InModeName == TEXT("")) return nullptr;

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
	GTCapturer->CaptureComponent = NewObject<USceneCaptureComponent2D>(); 
	// CaptureComponent needs to be attached to somewhere immediately, otherwise it will be gc-ed
	GTCapturer->CaptureComponent->AttachTo(InPawn->GetRootComponent());
	InitCaptureComponent(GTCapturer->CaptureComponent);

	GTCapturer->bIsTicking = true;

	UMaterial* Material = LoadMaterial(Mode);
	check(Material);
	if (Material)
	{
		GTCapturer->CaptureComponent->PostProcessSettings.AddBlendable(Material, 1);
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
	if (!bIsTicking)
	{
		bIsTicking = true;
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

	bIsTicking = false; // Only tick once.
}