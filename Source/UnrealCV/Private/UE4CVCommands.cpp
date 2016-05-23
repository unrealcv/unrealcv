// Fill out your copyright notice in the Description page of Project Settings.

// #include "RealisticRendering.h"
#include "UnrealCVPrivate.h"
#include "UE4CVCommands.h"
#include "ViewMode.h"
#include "ObjectPainter.h"

UE4CVCommands::UE4CVCommands(APawn* InCharacter, FCommandDispatcher* InCommandDispatcher)
{
	this->Character = InCharacter;
	this->CommandDispatcher = InCommandDispatcher;
	this->RegisterCommands();
}

UE4CVCommands::~UE4CVCommands()
{
}

void UE4CVCommands::RegisterCommands()
{
	this->RegisterCommandsCamera();
	// First version
	// CommandDispatcher->BindCommand("vset /mode/(?<ViewMode>.*)", SetViewMode); // Better to check the correctness at compile time
	FDispatcherDelegate Cmd;
	FString URI;
	// The regular expression for float number is from here, http://stackoverflow.com/questions/12643009/regular-expression-for-floating-point-numbers
	// Use ICU regexp to define URI, See http://userguide.icu-project.org/strings/regexp

	Cmd = FDispatcherDelegate::CreateRaw(&FViewMode::Get(), &FViewMode::SetMode);
	URI = "vset /mode/[str]";
	CommandDispatcher->BindCommand(URI, Cmd, "Set mode"); // Better to check the correctness at compile time

	Cmd = FDispatcherDelegate::CreateRaw(&FViewMode::Get(), &FViewMode::GetMode);
	CommandDispatcher->BindCommand("vget /mode", Cmd, "Get mode");

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
	FObjectPainter::Get().ObjectsColorMapping.GetKeys(Keys);
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
		TMap<FString, AActor*>& ObjectsMapping = FObjectPainter::Get().ObjectsMapping;
		TMap<FString, FColor>& ObjectsColorMapping = FObjectPainter::Get().ObjectsColorMapping;
		if (ObjectsMapping.Contains(ObjectName))
		{
			AActor* Actor = ObjectsMapping[ObjectName];
			if (FObjectPainter::Get().PaintObject(Actor, NewColor))
			{
				ObjectsColorMapping.Emplace(ObjectName, NewColor);
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

		TMap<FString, FColor>& ObjectsColorMapping = FObjectPainter::Get().ObjectsColorMapping;
		if (ObjectsColorMapping.Contains(ObjectName))
		{
			FColor ObjectColor = ObjectsColorMapping[ObjectName]; // Make sure the object exist
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
			// Character->ConsoleOutputDevice->Log(TraceString);
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

