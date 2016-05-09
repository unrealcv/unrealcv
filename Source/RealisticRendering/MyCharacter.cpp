// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "MyCharacter.h"
#include "MyHUD.h"
#include "StaticMeshResources.h"
#include "Networking.h"
#include <string>
#include "ImageUtils.h"
#include "ViewMode.h"
// #include "Console.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
	/*
	UMaterial* Material = LoadObject<UMaterial>(NULL, TEXT("/Game/TestMaterial.TestMaterial")); // The content needs to be cooked.
	static ConstructorHelpers::FObjectFinder<UTexture2D> CrosshairTexObj(TEXT("/Game/FirstPersonCrosshair"));
	static ConstructorHelpers::FObjectFinder<UMaterial> TestMaterial(TEXT("/Game/TestMaterial.TestMaterial"));
	static ConstructorHelpers::FObjectFinder<UMaterial> TestMaterial1(TEXT("/Game/TestMaterial"));
	UTexture2D* CrosshairTex = CrosshairTexObj.Object;
	UTexture2D* CrosshairTex1 = LoadObject<UTexture2D>(NULL, TEXT("/Game/FirstPersonCrosshair"));
	*/

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	FViewMode::World = this->GetWorld();
	Server = new FUE4CVServer(&CommandDispatcher, this->GetWorld());

	ConsoleOutputDevice = new FConsoleOutputDevice(GetWorld()->GetGameViewport()->ViewportConsole); // TODO: Check the pointers
	ConsoleHelper = new FConsoleHelper(&CommandDispatcher, ConsoleOutputDevice);

	DefineConsoleCommands();
	RegisterCommands();
	Server->Start();

	PaintRandomColors(TArray<FString>());
}

void AMyCharacter::NotifyClient(FString Message)
{
	// Send a message to client to say a new frame is rendered.
	Server->SendClientMessage(Message);
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}

void AMyCharacter::TakeScreenShot()
{
	FExecStatus ExecStatus = FExecStatus::OK;
	ExecStatus = CommandDispatcher.Exec("vget /camera/0/image");
	NotifyClient(ExecStatus.Message);
	ExecStatus = CommandDispatcher.Exec("vget /camera/0/location");
	NotifyClient(ExecStatus.Message);
	ExecStatus = CommandDispatcher.Exec("vget /camera/0/rotation");
	NotifyClient(ExecStatus.Message);
}

// Called every frame
void AMyCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	check(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);

	// Handle Mouse Input
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AMyCharacter::OnFire);
}

FExecStatus AMyCharacter::BindAxisWrapper(const TArray<FString>& Args)
{
	// FInputAxisHandlerSignature::TUObjectMethodDelegate< UserClass >::FMethodPtr
	return FExecStatus::OK;
}

void AMyCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AMyCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AMyCharacter::ParseMaterialConfiguration()
{
	// This testing function is from BufferVisualizationData
	FConfigSection* MaterialSection = GConfig->GetSectionPrivate(TEXT("Engine.BufferVisualizationMaterials"), false, true, GEngineIni);

	if (MaterialSection != NULL)
	{
		for (FConfigSection::TIterator It(*MaterialSection); It; ++It)
		{
			FString MaterialName;
			if (FParse::Value(*It.Value(), TEXT("Material="), MaterialName, true))
			{
				ConsoleOutputDevice->Log(MaterialName);
				UMaterial* Material = LoadObject<UMaterial>(NULL, *MaterialName);

				if (Material)
				{
					Material->AddToRoot(); // Prevent GC
					/*
					Record& Rec = MaterialMap.Add(It.Key(), Record());
					Rec.Name = It.Key().GetPlainNameString();
					Rec.Material = Material;
					FText DisplayName;
					FParse::Value(*It.Value(), TEXT("Name="), DisplayName, TEXT("Engine.BufferVisualizationMaterials"));
					Rec.DisplayName = DisplayName;
					*/
				}
			}
		}
	}

}

void AMyCharacter::TestMaterialLoading()
{
	UMaterial* Material = LoadObject<UMaterial>(NULL, TEXT("/Game/TestMaterial.TestMaterial")); // The content needs to be cooked.
}

void AMyCharacter::OnFire()
{
	// PaintAllObjects(TArray<FString>());
	// TakeScreenShot();

	// ParseMaterialConfiguration();
	// TestMaterialLoading();

	/*
	UE_LOG(LogTemp, Warning, TEXT("Fire"));
	FHitResult HitResult;
	// The original version for the shooting game use CameraComponent
	FVector StartLocation = GetActorLocation();
	FRotator Direction = GetActorRotation();

	FVector EndLocation = StartLocation + Direction.Vector() * 10000;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
	{
		AActor* HitActor = HitResult.GetActor();

		// UE_LOG(LogTemp, Warning, TEXT("%s"), *HitActor->GetActorLabel());
		// Draw a bounding box of the hitted object and also output the name of it.
	}
	*/
}

FExecStatus AMyCharacter::PaintRandomColors(const TArray<FString>& Args)
{
	// Iterate over all actors
	ULevel* Level = GetLevel();

	// Get a random color
	for (auto Actor : Level->Actors)
	{
		if (Actor && Actor->IsA(AStaticMeshActor::StaticClass())) // Only StaticMeshActor is interesting
		{
			// FString ActorLabel = Actor->GetActorLabel();
			FString ActorLabel = Actor->GetHumanReadableName();
			FColor NewColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
			ObjectsColorMapping.Emplace(ActorLabel, NewColor);
			// if (Actor->GetActorLabel() == FString("SM_Door43"))
			{
				PaintObject(Actor, NewColor);
			}
		}
	}

	// Paint actor using floodfill.
	return FExecStatus::OK;
}

bool AMyCharacter::PaintObject(AActor* Actor, const FColor& NewColor)
{
	if (!Actor) return false;

	TArray<UMeshComponent*> PaintableComponents;
	// TInlineComponentArray<UMeshComponent*> MeshComponents;
	// Actor->GetComponents<UMeshComponent>(MeshComponents);
	Actor->GetComponents<UMeshComponent>(PaintableComponents);


	for (auto MeshComponent : PaintableComponents)
	{
		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			if (UStaticMesh* StaticMesh = StaticMeshComponent->StaticMesh)
			{
				uint32 PaintingMeshLODIndex = 0;
				uint32 NumLODLevel = StaticMeshComponent->StaticMesh->RenderData->LODResources.Num();
				check(NumLODLevel == 1);
				FStaticMeshLODResources& LODModel = StaticMeshComponent->StaticMesh->RenderData->LODResources[PaintingMeshLODIndex];
				FStaticMeshComponentLODInfo* InstanceMeshLODInfo = NULL;

				// PaintingMeshLODIndex + 1 is the minimum requirement, enlarge if not satisfied
				StaticMeshComponent->SetLODDataCount(PaintingMeshLODIndex + 1, StaticMeshComponent->LODData.Num());
				InstanceMeshLODInfo = &StaticMeshComponent->LODData[PaintingMeshLODIndex];

				// Setup OverrideVertexColors
				if (!InstanceMeshLODInfo->OverrideVertexColors) {
					InstanceMeshLODInfo->OverrideVertexColors = new FColorVertexBuffer;

					FColor FillColor = FColor(255, 255, 255, 255);
					InstanceMeshLODInfo->OverrideVertexColors->InitFromSingleColor(FColor::White, LODModel.GetNumVertices());
				}

				uint32 NumVertices = LODModel.GetNumVertices();
				check(InstanceMeshLODInfo->OverrideVertexColors);
				check(NumVertices <= InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices());
				// StaticMeshComponent->CachePaintedDataIfNecessary();

				for (uint32 ColorIndex = 0; ColorIndex < NumVertices; ++ColorIndex)
				{
					// FColor NewColor = FColor(FMath::RandRange(0, 255), FMath::RandRange(0, 255), FMath::RandRange(0, 255), 255);
					// LODModel.ColorVertexBuffer.VertexColor(ColorIndex) = NewColor;  // This is vertex level
					// Need to initialize the vertex buffer first
					uint32 NumOverrideVertexColors = InstanceMeshLODInfo->OverrideVertexColors->GetNumVertices();
					uint32 NumPaintedVertices = InstanceMeshLODInfo->PaintedVertices.Num();
					// check(NumOverrideVertexColors == NumPaintedVertices);
					InstanceMeshLODInfo->OverrideVertexColors->VertexColor(ColorIndex) = NewColor;
					// InstanceMeshLODInfo->PaintedVertices[ColorIndex].Color = NewColor;
				}
				BeginInitResource(InstanceMeshLODInfo->OverrideVertexColors);
				StaticMeshComponent->MarkRenderStateDirty();
				// BeginUpdateResourceRHI(InstanceMeshLODInfo->OverrideVertexColors);


				/*
				// TODO: Need to check other LOD levels
				// Use flood fill to paint mesh vertices
				UE_LOG(LogTemp, Warning, TEXT("%s:%s has %d vertices"), *Actor->GetActorLabel(), *StaticMeshComponent->GetName(), NumVertices);

				if (LODModel.ColorVertexBuffer.GetNumVertices() == 0)
				{
					// Mesh doesn't have a color vertex buffer yet!  We'll create one now.
					LODModel.ColorVertexBuffer.InitFromSingleColor(FColor(255, 255, 255, 255), LODModel.GetNumVertices());
				}

				*/
			}
		}
	}
	return true;
}

FExecStatus AMyCharacter::SetCameraLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 4) // ID, X, Y, Z
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
		FVector Location = FVector(X, Y, Z);
		SetActorLocation(Location);

		return FExecStatus::OK;
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus AMyCharacter::SetCameraRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 4) // ID, Pitch, Roll, Yaw
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
		FRotator Rotator = FRotator(Pitch, Yaw, Roll);
		AController* Controller = GetController();
		Controller->ClientSetRotation(Rotator); // Teleport action
		// SetActorRotation(Rotator);  // This is not working

		return FExecStatus::OK;
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus AMyCharacter::GetCameraRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		// FRotator CameraRotation = GetActorRotation();  // We need the rotation of the controller
		FRotator CameraRotation = GetControlRotation();
		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);

		return FExecStatus(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}


FExecStatus AMyCharacter::GetCameraLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		FVector CameraLocation = GetActorLocation();
		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraLocation.X, CameraLocation.Y, CameraLocation.Z);

		return FExecStatus(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}

FExecStatus AMyCharacter::GetCameraImage(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]);

		static uint32 NumCaptured = 0;
		NumCaptured++;
		FString Filename = FString::Printf(TEXT("%04d.png"), NumCaptured);
		TArray<FColor> Bitmap;
		bool bScreenshotSuccessful = false;
		UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
		FViewport* InViewport = ViewportClient->Viewport;
		bScreenshotSuccessful = GetViewportScreenShot(InViewport, Bitmap);
		// ViewportClient->GetHighResScreenshotConfig().MergeMaskIntoAlpha(Bitmap);
		// Ensure that all pixels' alpha is set to 255
		for (auto& Color : Bitmap)
		{
			Color.A = 255;
		}

		if (bScreenshotSuccessful)
		{
			FIntVector Size(InViewport->GetSizeXY().X, InViewport->GetSizeXY().Y, 0);
			// TODO: Need to blend alpha, a bit weird from screen.

			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(Size.X, Size.Y, Bitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *Filename);
			return FExecStatus(Filename);
		}
		else
		{
			return FExecStatus::Error("Fail to capture image");
		}
	}
	return FExecStatus::InvalidArgument;
}

void AMyCharacter::RegisterCommands()
{
	// First version
	// CommandDispatcher.BindCommand("vset /mode/(?<ViewMode>.*)", SetViewMode); // Better to check the correctness at compile time
	FDispatcherDelegate Cmd;
	FString URI, Any = "(.*)", UInt = "(\\d*)", Float = "([-+]?\\d*[.]?\\d+)"; // Each type will be considered as a group
	// The regular expression for float number is from here, http://stackoverflow.com/questions/12643009/regular-expression-for-floating-point-numbers

	Cmd = FDispatcherDelegate::CreateStatic(FViewMode::SetMode);
	// Use ICU regexp to define URI, See http://userguide.icu-project.org/strings/regexp
	URI = FString::Printf(TEXT("vset /mode/%s"), *Any);
	CommandDispatcher.BindCommand(URI, Cmd); // Better to check the correctness at compile time

	Cmd = FDispatcherDelegate::CreateStatic(FViewMode::GetMode);
	CommandDispatcher.BindCommand("vget /mode", Cmd);

	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::GetCameraLocation);
	CommandDispatcher.BindCommand("vget /camera/(\\d*)/location", Cmd);
	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::GetCameraImage);
	CommandDispatcher.BindCommand("vget /camera/(\\d*)/image", Cmd); // Take a screenshot and return filename
	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::GetCameraRotation);
	CommandDispatcher.BindCommand("vget /camera/(\\d*)/rotation", Cmd);
	// CommandDispatcher.BindCommand("vget /camera/[id]/name", Command);

	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::SetCameraLocation);
	URI = FString::Printf(TEXT("vset /camera/%s/location %s %s %s"), *UInt, *Float, *Float, *Float);
	// TODO: Would be better if the format string can support named key
	CommandDispatcher.BindCommand(URI, Cmd);
	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::SetCameraRotation);
	URI = FString::Printf(TEXT("vset /camera/%s/rotation %s %s %s"), *UInt, *Float, *Float, *Float); // Pitch, Yaw, Roll
	CommandDispatcher.BindCommand(URI, Cmd);
	// CommandDispatcher.BindCommand("vset /camera/[id]/rotation [x] [y] [z]", Command);

	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::GetObjects);
	CommandDispatcher.BindCommand(TEXT("vget /objects"), Cmd);

	// The order matters
	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::CurrentObjectHandler); // Redirect to current 
	CommandDispatcher.BindCommand(TEXT("(.*) /object/_/(.*)"), Cmd);

	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::GetObjectColor);
	CommandDispatcher.BindCommand(TEXT("vget /object/(.*)/color"), Cmd);

	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::SetObjectColor);
	CommandDispatcher.BindCommand(TEXT("vset /object/(.*)/color"), Cmd);

	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::GetObjectName);
	CommandDispatcher.BindCommand(TEXT("vget /object/(.*)/name"), Cmd);

	Cmd = FDispatcherDelegate::CreateUObject(this, &AMyCharacter::PaintRandomColors);
	CommandDispatcher.BindCommand(TEXT("vset /util/paint_object"), Cmd);

	CommandDispatcher.Alias("VisionDepth", "vset /mode/depth"); // Alias for human interaction
	CommandDispatcher.Alias("VisionCamInfo", "vget /camera/0/name");
}

FExecStatus AMyCharacter::GetObjects(const TArray<FString>& Args)
{
	TArray<FString> Keys;
	ObjectsColorMapping.GetKeys(Keys);
	FString Message = "";
	for (auto ObjectName : Keys)
	{
		Message += ObjectName + " ";
	}
	return FExecStatus(Message);
}

FExecStatus AMyCharacter::SetObjectColor(const TArray<FString>& Args)
{
	// ObjectName, R, G, B, A
	// The color format is RGBA
	if (Args.Num() == 5)
	{ 
		FString ObjectName = Args[0];
		uint32 R = FCString::Atoi(*Args[1]), G = FCString::Atoi(*Args[2]), B = FCString::Atoi(*Args[3]), A = FCString::Atoi(*Args[4]);
		FColor NewColor(R, G, B, A);
		if (ObjectsMapping.Contains(ObjectName))
		{
			AActor* Actor = ObjectsMapping[ObjectName];
			if (PaintObject(Actor, NewColor))
			{
				ObjectsColorMapping.Emplace(ObjectName, NewColor);
				return FExecStatus::OK;
			}
			else
			{
				return FExecStatus::Error(FString::Printf(TEXT("Failed to paint object %s"), *ObjectName));
			}
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Object %s not exist"), *ObjectName));
		}
	}

	return FExecStatus::InvalidArgument;
}


FExecStatus AMyCharacter::GetObjectColor(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{ 
		FString ObjectName = Args[0];

		if (ObjectsColorMapping.Contains(ObjectName))
		{
			FColor ObjectColor = ObjectsColorMapping[ObjectName]; // Make sure the object exist
			FString Message = ObjectColor.ToString();
			// FString Message = "%.3f %.3f %.3f %.3f";
			return FExecStatus(Message);
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Object %s not exist"), *ObjectName));
		}
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus AMyCharacter::GetObjectName(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		return FExecStatus(Args[0]);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus AMyCharacter::CurrentObjectHandler(const TArray<FString>& Args)
{
	// At least one parameter
	if (Args.Num() >= 2)
	{
		FString Uri = "";
		// Get the name of current object
		FHitResult HitResult;
		// The original version for the shooting game use CameraComponent
		FVector StartLocation = GetActorLocation();
		// FRotator Direction = GetActorRotation();
		FRotator Direction = GetControlRotation();

		FVector EndLocation = StartLocation + Direction.Vector() * 10000;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController != nullptr)
		{
			FHitResult TraceResult(ForceInit);
			PlayerController->GetHitResultUnderCursor(ECollisionChannel::ECC_WorldDynamic, false, TraceResult);
			FString TraceString;
			if (TraceResult.GetActor() != nullptr)
			{
				TraceString += FString::Printf(TEXT("Trace Actor %s."), *TraceResult.GetActor()->GetName());
			}
			if (TraceResult.GetComponent() != nullptr)
			{
				TraceString += FString::Printf(TEXT("Trace Comp %s."), *TraceResult.GetComponent()->GetName());
			}
			// TheHud->TraceResultText = TraceString;
			ConsoleOutputDevice->Log(TraceString);
		}
		// TODO: This is not working well.

		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();

			// UE_LOG(LogTemp, Warning, TEXT("%s"), *HitActor->GetActorLabel());
			// Draw a bounding box of the hitted object and also output the name of it.
			FString ActorName = HitActor->GetHumanReadableName();
			FString Method = Args[0], Property = Args[1];
			Uri = FString::Printf(TEXT("%s /object/%s/%s"), *Method, *ActorName, *Property); // Method name

			for (int32 ArgIndex = 2; ArgIndex < Args.Num(); ArgIndex++) // Vargs
			{
				Uri += " " + Args[ArgIndex];
			}
			FExecStatus ExecStatus = CommandDispatcher.Exec(Uri);
			return ExecStatus;
		}
		else
		{
			return FExecStatus::Error("Can not find current object");
		}
	}
	return FExecStatus::InvalidArgument;
}

void AMyCharacter::DefineConsoleCommands()
{
	// Select An Object, this is done through OnFire event
	// ViewMode
	FViewMode::RegisterCommands();

	// Show and Hide the Cursor
	IConsoleObject* ToggleCursorCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("VisionToggleCursor"),
		TEXT("Toggle whether the cursor is visible"),
		FConsoleCommandDelegate::CreateStatic(AMyHUD::ToggleCursor),
		ECVF_Default
		);

	// Show labels on the screen
	IConsoleObject* ToggleLabelCmd = IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("VisionToggleLabel"),
		TEXT("Toggle whether the label of object is visible"),
		FConsoleCommandDelegate::CreateStatic(AMyHUD::ToggleLabel),
		ECVF_Default
		);

}
