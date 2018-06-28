// Weichao Qiu @ 2017
#include "UnrealcvWorldController.h"

#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/Pawn.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"

#include "Sensor/CameraSensor/PawnCamSensor.h"
#include "VisionBPLib.h"
#include "UnrealcvServer.h"
#include "PlayerViewMode.h"
#include "UnrealcvLog.h"

AUnrealcvWorldController::AUnrealcvWorldController(const FObjectInitializer& ObjectInitializer)
{
	PlayerViewMode = CreateDefaultSubobject<UPlayerViewMode>(TEXT("PlayerViewMode"));
}

void AttachPawnSensor(APawn* Pawn)
{
	if (!IsValid(Pawn))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The pawn is invalid, can not attach a PawnSensor to it."));
		return;
	}

	UE_LOG(LogUnrealCV, Display, TEXT("Attach a UnrealcvSensor to the pawn"));
	// Make sure this is the first one.
	UPawnCamSensor* PawnCamSensor = NewObject<UPawnCamSensor>(Pawn, TEXT("PawnSensor")); // Make Pawn as the owner of the component
	// UFusionCamSensor* FusionCamSensor = ConstructObject<UFusionCamSensor>(UFusionCamSensor::StaticClass(), Pawn);

	UWorld *PawnWorld = Pawn->GetWorld(), *GameWorld = FUnrealcvServer::Get().GetGameWorld();
	// check(Pawn->GetWorld() == FUnrealcvServer::GetGameWorld());
	check(PawnWorld == GameWorld);
	PawnCamSensor->AttachToComponent(Pawn->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	// AActor* OwnerActor = FusionCamSensor->GetOwner();
	PawnCamSensor->RegisterComponent(); // Is this neccessary?
}



void AUnrealcvWorldController::BeginPlay()
{
	UE_LOG(LogUnrealCV, Display, TEXT("Overwrite the world setting with some UnrealCV extensions"));

	FUnrealcvServer& UnrealcvServer = FUnrealcvServer::Get();
	if (UnrealcvServer.TcpServer && !UnrealcvServer.TcpServer->IsListening())
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The tcp server is not running"));
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	check(PlayerController);
	APawn* Pawn = PlayerController->GetPawn();
	FUnrealcvServer& Server = FUnrealcvServer::Get();
	if (IsValid(Pawn))
	{
		AttachPawnSensor(Pawn);
		UVisionBPLib::UpdateInput(Pawn, Server.Config.EnableInput);
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("No available pawn to mount a PawnSensor"));
	}

	ObjectAnnotator.AnnotateWorld(GetWorld());

	FEngineShowFlags ShowFlags = GetWorld()->GetGameViewport()->EngineShowFlags;
	this->PlayerViewMode->SaveGameDefault(ShowFlags);

	// TODO: remove legacy code
	// Update camera FOV

	//PlayerController->PlayerCameraManager->SetFOV(Server.Config.FOV);
	//FCaptureManager::Get().AttachGTCaptureComponentToCamera(Pawn);
}



void AUnrealcvWorldController::OpenLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::OpenLevel(World, LevelName);
	UGameplayStatics::FlushLevelStreaming(World);
	UE_LOG(LogUnrealCV, Warning, TEXT("Level loaded"));
}
