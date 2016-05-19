// Fill out your copyright notice in the Description page of Project Settings.

#include "RealisticRendering.h"
#include "UE4CVCommands.h"
#include "MyGameViewportClient.h"
#include "ViewMode.h"
#include "ImageUtils.h"

UE4CVCommands::UE4CVCommands(AMyCharacter* InCharacter)
{
	this->Character = InCharacter;
	this->CommandDispatcher = &InCharacter->CommandDispatcher;
	this->RegisterCommands();
}

UE4CVCommands::~UE4CVCommands()
{
}

void UE4CVCommands::RegisterCommands()
{
	// First version
	// CommandDispatcher->BindCommand("vset /mode/(?<ViewMode>.*)", SetViewMode); // Better to check the correctness at compile time
	FDispatcherDelegate Cmd;
	FString URI;
	// The regular expression for float number is from here, http://stackoverflow.com/questions/12643009/regular-expression-for-floating-point-numbers
	// Use ICU regexp to define URI, See http://userguide.icu-project.org/strings/regexp

	Cmd = FDispatcherDelegate::CreateStatic(FViewMode::SetMode);
	URI = "vset /mode/[str]";
	CommandDispatcher->BindCommand(URI, Cmd, "Set mode"); // Better to check the correctness at compile time

	Cmd = FDispatcherDelegate::CreateStatic(FViewMode::GetMode);
	CommandDispatcher->BindCommand("vget /mode", Cmd, "Get mode");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraLocation);
	CommandDispatcher->BindCommand("vget /camera/[uint]/location", Cmd, "Get camera location");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraImage);
	CommandDispatcher->BindCommand("vget /camera/[uint]/image", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraView);
	CommandDispatcher->BindCommand("vget /camera/[uint]/view", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraRotation);
	CommandDispatcher->BindCommand("vget /camera/[uint]/rotation", Cmd, "Get camera rotation");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraDepth);
	CommandDispatcher->BindCommand("vget /camera/[uint]/depth", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCameraObjectMask);
	CommandDispatcher->BindCommand("vget /camera/[uint]/object_mask", Cmd, "Get snapshot from camera"); // Take a screenshot and return filename

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::SetCameraLocation);
	URI = "vset /camera/[uint]/location [float] [float] [float]";
	CommandDispatcher->BindCommand(URI, Cmd, "Set camera location");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::SetCameraRotation);
	URI = "vset /camera/[uint]/rotation [float] [float] [float]"; // Pitch, Yaw, Roll
	CommandDispatcher->BindCommand(URI, Cmd, "Set camera rotation");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetObjects);
	CommandDispatcher->BindCommand(TEXT("vget /objects"), Cmd, "Get all objects in the scene");

	// The order matters
	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::CurrentObjectHandler); // Redirect to current 
	CommandDispatcher->BindCommand(TEXT("[str] /object/_/[str]"), Cmd, "Get current object");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetObjectColor);
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/color"), Cmd, "Get object color");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::SetObjectColor);
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/color"), Cmd, "Set object color");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetObjectName);
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/name"), Cmd, "Get object name");

	// Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::PaintRandomColors);
	// CommandDispatcher->BindCommand(TEXT("vget /util/random_paint"), Cmd, "Paint objects with random color");

	Cmd = FDispatcherDelegate::CreateRaw(this, &UE4CVCommands::GetCommands);
	CommandDispatcher->BindCommand(TEXT("vget /util/get_commands"), Cmd, "Get all available commands");

	CommandDispatcher->Alias("SetDepth", "vset /mode/depth", "Set mode to depth"); // Alias for human interaction
	CommandDispatcher->Alias("VisionCamInfo", "vget /camera/0/name", "Get camera info");
	CommandDispatcher->Alias("ls", "vget /util/get_commands", "List all commands");
	CommandDispatcher->Alias("shot", "vget /camera/0/image", "Save image to disk");

}


FExecStatus UE4CVCommands::SetCameraLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 4) // ID, X, Y, Z
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
		FVector Location = FVector(X, Y, Z);
		Character->SetActorLocation(Location);

		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::SetCameraRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 4) // ID, Pitch, Roll, Yaw
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
		FRotator Rotator = FRotator(Pitch, Yaw, Roll);
		AController* Controller = Character->GetController();
		Controller->ClientSetRotation(Rotator); // Teleport action
		// SetActorRotation(Rotator);  // This is not working

		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::GetCameraRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		// FRotator CameraRotation = GetActorRotation();  // We need the rotation of the controller
		FRotator CameraRotation = Character->GetControlRotation();
		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraRotation.Pitch, CameraRotation.Yaw, CameraRotation.Roll);

		return FExecStatus::OK(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}

FExecStatus UE4CVCommands::GetCameraDepth(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FExecStatus ExecStatus = CommandDispatcher->Exec("vset /mode/depth"); // TODO: Overwrite the + operator of FExecStatus
		return GetCameraView(Args);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::GetCameraObjectMask(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FExecStatus ExecStatus = CommandDispatcher->Exec("vset /mode/object"); // TODO: Overwrite the + operator of FExecStatus
		return GetCameraView(Args);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::GetCameraImage(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FExecStatus ExecStatus = CommandDispatcher->Exec("vset /mode/lit");
		return GetCameraView(Args);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::GetCameraLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]); // TODO: Add support for multiple cameras
		FVector CameraLocation = Character->GetActorLocation();
		FString Message = FString::Printf(TEXT("%.3f %.3f %.3f"), CameraLocation.X, CameraLocation.Y, CameraLocation.Z);

		return FExecStatus::OK(Message);
	}
	return FExecStatus::Error("Number of arguments incorrect");
}

// Sync operation for screen capture
bool DoCaptureScreen(UGameViewportClient *ViewportClient, const FString& CaptureFilename)
{
		TArray<FColor> Bitmap;
		bool bScreenshotSuccessful = false;
		FViewport* InViewport = ViewportClient->Viewport;
		ViewportClient->GetEngineShowFlags()->SetMotionBlur(false);
		bScreenshotSuccessful = GetViewportScreenShot(InViewport, Bitmap);

		if (bScreenshotSuccessful)
		{
			// Ensure that all pixels' alpha is set to 255
			for (auto& Color : Bitmap)
			{
				Color.A = 255;
			}
			FIntVector Size(InViewport->GetSizeXY().X, InViewport->GetSizeXY().Y, 0);
			// TODO: Need to blend alpha, a bit weird from screen.

			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(Size.X, Size.Y, Bitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *CaptureFilename);
		}
		return bScreenshotSuccessful;
}

FExecStatus UE4CVCommands::GetCameraViewSync(const FString& FullFilename)
{
	// This can only work within editor
	// Reimplement a GameViewportClient is required according to the discussion from here
	// https://forums.unrealengine.com/showthread.php?50857-FViewPort-ReadPixels-crash-while-play-on-quot-standalone-Game-quot-mode
	UGameViewportClient* ViewportClient = Character->GetWorld()->GetGameViewport();
	if (DoCaptureScreen(ViewportClient, FullFilename))
	{
		return FExecStatus::OK(FullFilename);
	}
	else
	{
		return FExecStatus::Error("Fail to capture screen");
	}
}

FExecStatus UE4CVCommands::GetCameraViewAsyncQuery(const FString& FullFilename)
{
		// Method 1: Use custom ViewportClient
		UMyGameViewportClient* ViewportClient = (UMyGameViewportClient*)Character->GetWorld()->GetGameViewport();
		ViewportClient->CaptureScreen(FullFilename);

		// Method2: System screenshot function
		FScreenshotRequest::RequestScreenshot(FullFilename, false, false); // This is an async operation

		// Implement 2, Start async and query
		// FPromiseDelegate PromiseDelegate = FPromiseDelegate::CreateRaw(this, &UE4CVCommands::CheckStatusScreenshot);
		FPromiseDelegate PromiseDelegate = FPromiseDelegate::CreateLambda([FullFilename]()
		{
			if (FScreenshotRequest::IsScreenshotRequested()) return FExecStatus::Pending();
			else return FExecStatus::OK(FullFilename);
		});
		FExecStatus ExecStatusQuery = FExecStatus::AsyncQuery(FPromise(PromiseDelegate));
		/*
		UGameViewportClient* ViewportClient = Character->GetWorld()->GetGameViewport();
		ViewportClient->OnScreenshotCaptured().Clear(); // This is required to handle the filename issue.
		ViewportClient->OnScreenshotCaptured().AddLambda(
			[FullFilename](int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap) 
		{
			// Save bitmap to disk
			TArray<FColor>& RefBitmap = const_cast<TArray<FColor>&>(Bitmap);
			for (auto& Color : RefBitmap)
			{
				Color.A = 255;
			}

			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(SizeX, SizeY, RefBitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *FullFilename);
		});
		*/
		return ExecStatusQuery;

}

FExecStatus UE4CVCommands::GetCameraViewAsyncCallback(const FString& FullFilename)
{
		FScreenshotRequest::RequestScreenshot(FullFilename, false, false); // This is an async operation

		// async implement 1, Start async and callback
		FExecStatus ExecStatusAsyncCallback = FExecStatus::AsyncCallback();
		int32 TaskId = ExecStatusAsyncCallback.TaskId;
		UMyGameViewportClient* ViewportClient = (UMyGameViewportClient*)Character->GetWorld()->GetGameViewport();
		ViewportClient->OnScreenshotCaptured().Clear();
		ViewportClient->OnScreenshotCaptured().AddLambda(
			[FullFilename, TaskId](int32 SizeX, int32 SizeY, const TArray<FColor>& Bitmap) 
		{
			// Save bitmap to disk
			TArray<FColor>& RefBitmap = const_cast<TArray<FColor>&>(Bitmap);
			TArray<uint8> CompressedBitmap;
			FImageUtils::CompressImageArray(SizeX, SizeY, RefBitmap, CompressedBitmap);
			FFileHelper::SaveArrayToFile(CompressedBitmap, *FullFilename);

			FString Message = FullFilename;
			// FAsyncTaskPool::Get().CompleteTask(TaskId, Message); // int32 is easy to pass around in lamdba
			/*
			// Mark this async task finished
			ExecStatusAsyncCallback.MessageBody = FullFilename;
			ExecStatusAsyncCallback.OnFinished().Execute(); // FullFilename is the message to return
			*/
		});
		return ExecStatusAsyncCallback;
}


FExecStatus UE4CVCommands::GetCameraView(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		int32 CameraId = FCString::Atoi(*Args[0]);

		static uint32 NumCaptured = 0;
		NumCaptured++;
		FString Filename = FString::Printf(TEXT("%04d.png"), NumCaptured);
		const FString Dir = FString(FPlatformProcess::BaseDir()); // TODO: Change this to screen capture folder
		FString FullFilename = FPaths::Combine(*Dir, *Filename);
		
		return this->GetCameraViewAsyncCallback(FullFilename);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::GetCommands(const TArray<FString>& Args)
{
	FString Message;

	TArray<FString> UriList;
	TMap<FString, FString> UriDescription = CommandDispatcher->GetUriDescription();
	UriDescription.GetKeys(UriList);

	for (auto Value : UriDescription)
	{
		Message += Value.Key + "\n";
		Message += Value.Value + "\n";
	}

	return FExecStatus::OK(Message);
}

FExecStatus UE4CVCommands::GetObjects(const TArray<FString>& Args)
{
	TArray<FString> Keys;
	Character->ObjectsColorMapping.GetKeys(Keys);
	FString Message = "";
	for (auto ObjectName : Keys)
	{
		Message += ObjectName + " ";
	}
	return FExecStatus::OK(Message);
}

FExecStatus UE4CVCommands::SetObjectColor(const TArray<FString>& Args)
{
	// ObjectName, R, G, B, A
	// The color format is RGBA
	if (Args.Num() == 5)
	{ 
		FString ObjectName = Args[0];
		uint32 R = FCString::Atoi(*Args[1]), G = FCString::Atoi(*Args[2]), B = FCString::Atoi(*Args[3]), A = FCString::Atoi(*Args[4]);
		FColor NewColor(R, G, B, A);
		if (Character->ObjectsMapping.Contains(ObjectName))
		{
			AActor* Actor = Character->ObjectsMapping[ObjectName];
			if (Character->PaintObject(Actor, NewColor))
			{
				Character->ObjectsColorMapping.Emplace(ObjectName, NewColor);
				return FExecStatus::OK();
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


FExecStatus UE4CVCommands::GetObjectColor(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{ 
		FString ObjectName = Args[0];

		if (Character->ObjectsColorMapping.Contains(ObjectName))
		{
			FColor ObjectColor = Character->ObjectsColorMapping[ObjectName]; // Make sure the object exist
			FString Message = ObjectColor.ToString();
			// FString Message = "%.3f %.3f %.3f %.3f";
			return FExecStatus::OK(Message);
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Object %s not exist"), *ObjectName));
		}
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::GetObjectName(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		return FExecStatus::OK(Args[0]);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus UE4CVCommands::CurrentObjectHandler(const TArray<FString>& Args)
{
	// At least one parameter
	if (Args.Num() >= 2)
	{
		FString Uri = "";
		// Get the name of current object
		FHitResult HitResult;
		// The original version for the shooting game use CameraComponent
		FVector StartLocation = Character->GetActorLocation();
		// FRotator Direction = GetActorRotation();
		FRotator Direction = Character->GetControlRotation();

		FVector EndLocation = StartLocation + Direction.Vector() * 10000;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character);

		APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
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
			Character->ConsoleOutputDevice->Log(TraceString);
		}
		// TODO: This is not working well.

		if (Character->GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
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
			FExecStatus ExecStatus = CommandDispatcher->Exec(Uri);
			return ExecStatus;
		}
		else
		{
			return FExecStatus::Error("Can not find current object");
		}
	}
	return FExecStatus::InvalidArgument;
}

