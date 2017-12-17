// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "ObjectHandler.h"
#include "ObjectPainter.h"
#include "ActorController.h"
#include "VertexSensor.h"

FExecStatus GetObjectMobility(const TArray<FString>& Args);

FExecStatus GetActorList(const TArray<FString>& Args);

AActor* GetActor(const TArray<FString>& Args);
FExecStatus GetActorLocation(const TArray<FString>& Args);
FExecStatus GetActorRotation(const TArray<FString>& Args);
FExecStatus GetActorVertexLocation(const TArray<FString>& Args);
FExecStatus GetActorVertexColor(const TArray<FString>& Args);
FExecStatus SetActorVertexColor(const TArray<FString>& Args);

void FObjectCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;
	FString Help;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjects);
	Help = "Get the name of all objects";
	CommandDispatcher->BindCommand(TEXT("vget /objects"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectLocation);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/location",
		FDispatcherDelegate::CreateStatic(GetActorLocation),
		"Get object location [x, y, z]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/rotation",
		FDispatcherDelegate::CreateStatic(GetActorRotation),
		"Get object rotation [pitch, yaw, roll]"
	);

	CommandDispatcher->BindCommand(
		"vget /object/[str]/vertex_location",
		FDispatcherDelegate::CreateStatic(GetActorVertexLocation),
		"Get vertex location"
	);

	// Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetObjectLocation);
	// Help = "Set object location [x, y, z]";
	// CommandDispatcher->BindCommand(TEXT("vset /object/[str]/location [float] [float] [float]"), Cmd, Help);

	// Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetObjectRotation);
	// Help = "Set object rotation [pitch, yaw, roll]";
	// CommandDispatcher->BindCommand(TEXT("vset /object/[str]/rotation [float] [float] [float]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectColor);
	Help = "Get the labeling color of an object (used in object instance mask)";
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/color"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetObjectColor);
	Help = "Set the labeling color of an object [r, g, b]";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/color [uint] [uint] [uint]"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectName);
	Help = "[debug] Get the object name";
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/name"), Cmd, Help);


	Cmd = FDispatcherDelegate::CreateStatic(GetObjectMobility);
	Help = "Is the object static or movable?";
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/mobility"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::ShowObject);
	Help = "Show object";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/show"), Cmd, Help);

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::HideObject);
	Help = "Hide object";
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/hide"), Cmd, Help);
}

TArray<AActor*> GetActorPtrList(UWorld* World)
{
	TArray<UObject*> UObjectList;
	bool bIncludeDerivedClasses = true;
	EObjectFlags ExclusionFlags = EObjectFlags::RF_ClassDefaultObject;
	EInternalObjectFlags ExclusionInternalFlags = EInternalObjectFlags::AllFlags;
	GetObjectsOfClass(AActor::StaticClass(), UObjectList, bIncludeDerivedClasses, ExclusionFlags, ExclusionInternalFlags);

	TArray<AActor*> ActorList;
	for (UObject* ActorObject : UObjectList)
	{
		AActor* Actor = Cast<AActor>(ActorObject);
		if (Actor->GetWorld() != World) continue;
		if (ActorList.Contains(Actor) == false)
		{
			ActorList.Add(Actor);
		}
	}
	return ActorList;
}

FExecStatus GetActorList(const TArray<FString>& Args)
{
	UWorld* GameWorld = FUE4CVServer::Get().GetGameWorld();
	TArray<AActor*> ActorList = GetActorPtrList(GameWorld);

	FString StrActorList;
	for (AActor* Actor : ActorList)
	{
		StrActorList += FString::Printf(TEXT("%s "), *Actor->GetName());
	}
	return FExecStatus::OK();
}

AActor* GetActor(const TArray<FString>& Args)
{
	FString ActorId = Args[0];
	AActor* Actor = GetActorById(FUE4CVServer::Get().GetGameWorld(), ActorId);
	return Actor;
}

FExecStatus GetActorLocation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	FActorController Controller(Actor);
	return FExecStatus::OK(Controller.GetLocation().ToString());
}

FExecStatus GetActorRotation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	FActorController Controller(Actor);
	return FExecStatus::OK(Controller.GetRotation().ToString());
}

FExecStatus GetActorVertexLocation(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	FVertexSensor Sensor(Actor);
	TArray<FVector> VertexArray = Sensor.GetVertexArray();

	// Serialize it to json?
	FString Str = "";
	for (auto Vector : Data)
	{
		FString VertexLocation = FString::Printf(
			TEXT("%.5f     %.5f     %.5f"),
			Data.X, Data.Y, Data.Z);
		Str += VertexLocation + "\n";
	}

	return FExecStatus::OK(Str);
}

FExecStatus GetActorVertexColor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	FVertexSensor Sensor(Actor);
	TArray<FVector> VertexArray = Sensor.GetVertexArray();

	// Serialize it to json?
	FString Str = Serialize(VertexArray);

	return FExecStatus::OK(Str);
}

FExecStatus SetActorVertexColor(const TArray<FString>& Args)
{
	AActor* Actor = GetActor(Args);
	FVertexSensor Sensor(Actor);
	TArray<FVector> VertexArray = Sensor.GetVertexArray();

	// Serialize it to json?
	FString Str = Serialize(VertexArray);

	return FExecStatus::OK(Str);
}

FExecStatus FObjectCommandHandler::GetObjects(const TArray<FString>& Args)
{
	return FObjectPainter::Get().GetObjectList();
}

FExecStatus FObjectCommandHandler::SetObjectColor(const TArray<FString>& Args)
{
	// ObjectName, R, G, B, A = 255
	// The color format is RGBA
	if (Args.Num() == 4)
	{
		FString ObjectName = Args[0];
		uint32 R = FCString::Atoi(*Args[1]), G = FCString::Atoi(*Args[2]), B = FCString::Atoi(*Args[3]), A = 255; // A = FCString::Atoi(*Args[4]);
		FColor NewColor(R, G, B, A);

		return FObjectPainter::Get().SetActorColor(ObjectName, NewColor);
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetObjectColor(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		return FObjectPainter::Get().GetActorColor(ObjectName);
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetObjectName(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		return FExecStatus::OK(Args[0]);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::CurrentObjectHandler(const TArray<FString>& Args)
{
	// At least one parameter
	if (Args.Num() >= 2)
	{
		FString Uri = "";
		// Get the name of current object
		FHitResult HitResult;
		// The original version for the shooting game use CameraComponent
		APawn* Pawn = FUE4CVServer::Get().GetPawn();
		FVector StartLocation = Pawn->GetActorLocation();
		// FRotator Direction = GetActorRotation();
		FRotator Direction = Pawn->GetControlRotation();

		FVector EndLocation = StartLocation + Direction.Vector() * 10000;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Pawn);

		APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
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

		if (this->GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
		{
			AActor* HitActor = HitResult.GetActor();

			// UE_LOG(LogUnrealCV, Warning, TEXT("%s"), *HitActor->GetActorLabel());
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

FExecStatus FObjectCommandHandler::GetObjectLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		FVector Location = Object->GetActorLocation();
		return FExecStatus::OK(FString::Printf(TEXT("%.2f %.2f %.2f"), Location.X, Location.Y, Location.Z));
	}

	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::GetObjectRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		// TODO: support quaternion
		FRotator Rotation = Object->GetActorRotation();
		return FExecStatus::OK(FString::Printf(TEXT("%.2f %.2f %.2f"), Rotation.Pitch, Rotation.Yaw, Rotation.Roll));
	}
	return FExecStatus::InvalidArgument;
}

/** There is no guarantee this will always succeed, for example, hitting a wall */
FExecStatus FObjectCommandHandler::SetObjectLocation(const TArray<FString>& Args)
{
	if (Args.Num() == 4)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		// TODO: Check whether this object is movable
		float X = FCString::Atof(*Args[1]), Y = FCString::Atof(*Args[2]), Z = FCString::Atof(*Args[3]);
		FVector NewLocation = FVector(X, Y, Z);
		bool Success = Object->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

		if (Success)
		{
			return FExecStatus::OK();
		}
		else
		{
			return FExecStatus::Error(FString::Printf(TEXT("Failed to move object %s"), *ObjectName));
		}
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::SetObjectRotation(const TArray<FString>& Args)
{
	if (Args.Num() == 4)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		// TODO: Check whether this object is movable
		float Pitch = FCString::Atof(*Args[1]), Yaw = FCString::Atof(*Args[2]), Roll = FCString::Atof(*Args[3]);
		FRotator Rotator = FRotator(Pitch, Yaw, Roll);
		bool Success = Object->SetActorRotation(Rotator);
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus GetObjectMobility(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		FString MobilityName = "";
		EComponentMobility::Type Mobility = Object->GetRootComponent()->Mobility.GetValue();
		switch (Mobility)
		{
		case EComponentMobility::Type::Movable: MobilityName = "Movable"; break;
		case EComponentMobility::Type::Static: MobilityName = "Static"; break;
		case EComponentMobility::Type::Stationary: MobilityName = "Stationary"; break;
		default: MobilityName = "Unknown";
		}
		return FExecStatus::OK(MobilityName);
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::ShowObject(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		Object->SetActorHiddenInGame(false);
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}

FExecStatus FObjectCommandHandler::HideObject(const TArray<FString>& Args)
{
	if (Args.Num() == 1)
	{
		FString ObjectName = Args[0];
		AActor* Object = FObjectPainter::Get().GetObject(ObjectName);
		if (Object == NULL)
		{
			return FExecStatus::Error(FString::Printf(TEXT("Can not find object %s"), *ObjectName));
		}

		Object->SetActorHiddenInGame(true);
		return FExecStatus::OK();
	}
	return FExecStatus::InvalidArgument;
}
