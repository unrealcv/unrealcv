#include "UnrealCVPrivate.h"
#include "ObjectHandler.h"
#include "ObjectPainter.h"


void FObjectCommandHandler::RegisterCommands()
{
	FDispatcherDelegate Cmd;

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjects);
	CommandDispatcher->BindCommand(TEXT("vget /objects"), Cmd, "Get all objects in the scene");

	// The order matters
	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::CurrentObjectHandler); // Redirect to current
	CommandDispatcher->BindCommand(TEXT("[str] /object/_/[str]"), Cmd, "Get current object");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectColor);
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/color"), Cmd, "Get object color");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::SetObjectColor);
	CommandDispatcher->BindCommand(TEXT("vset /object/[str]/color [uint] [uint] [uint]"), Cmd, "Set object color");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectName);
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/name"), Cmd, "Get object name");

	Cmd = FDispatcherDelegate::CreateRaw(this, &FObjectCommandHandler::GetObjectLocation);
	CommandDispatcher->BindCommand(TEXT("vget /object/[str]/location"), Cmd, "Get object location");
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
