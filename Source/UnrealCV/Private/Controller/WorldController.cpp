// Weichao Qiu @ 2017
#include "WorldController.h"

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

void AUnrealcvWorldController::AttachPawnSensor()
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	check(PlayerController);
	APawn* Pawn = PlayerController->GetPawn();
	FUnrealcvServer& Server = FUnrealcvServer::Get();
	if (!IsValid(Pawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("No available pawn to mount a PawnSensor"));
		return;
	}

	UE_LOG(LogUnrealCV, Display, TEXT("Attach a UnrealcvSensor to the pawn"));
	// Make sure this is the first one.
	UPawnCamSensor* PawnCamSensor = NewObject<UPawnCamSensor>(Pawn, TEXT("PawnSensor")); // Make Pawn as the owner of the component
	// UFusionCamSensor* FusionCamSensor = ConstructObject<UFusionCamSensor>(UFusionCamSensor::StaticClass(), Pawn);

	UWorld *PawnWorld = Pawn->GetWorld(), *GameWorld = FUnrealcvServer::Get().GetWorld();
	// check(Pawn->GetWorld() == FUnrealcvServer::GetWorld());
	check(PawnWorld == GameWorld);
	PawnCamSensor->AttachToComponent(Pawn->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	// AActor* OwnerActor = FusionCamSensor->GetOwner();
	PawnCamSensor->RegisterComponent(); // Is this neccessary?
	UVisionBPLib::UpdateInput(Pawn, Server.Config.EnableInput);
}

void AUnrealcvWorldController::InitWorld()
{
	UE_LOG(LogUnrealCV, Display, TEXT("Overwrite the world setting with some UnrealCV extensions"));
	FUnrealcvServer& UnrealcvServer = FUnrealcvServer::Get();
	if (UnrealcvServer.TcpServer && !UnrealcvServer.TcpServer->IsListening())
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The tcp server is not running"));
	}


	ObjectAnnotator.AnnotateWorld(GetWorld());

	FEngineShowFlags ShowFlags = GetWorld()->GetGameViewport()->EngineShowFlags;
	this->PlayerViewMode->SaveGameDefault(ShowFlags);

	this->AttachPawnSensor();

	// TODO: remove legacy code
	// Update camera FOV

	//PlayerController->PlayerCameraManager->SetFOV(Server.Config.FOV);
	//FCaptureManager::Get().AttachGTCaptureComponentToCamera(Pawn);
}

void AUnrealcvWorldController::PostActorCreated()
{
	UE_LOG(LogTemp, Display, TEXT("AUnrealcvWorldController::PostActorCreated"));
	Super::PostActorCreated();
}


void AUnrealcvWorldController::BeginPlay()
{
	UE_LOG(LogTemp, Display, TEXT("AUnrealcvWorldController::BeginPlay"));
	Super::BeginPlay();
}



void AUnrealcvWorldController::OpenLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::OpenLevel(World, LevelName);
	UGameplayStatics::FlushLevelStreaming(World);
	UE_LOG(LogUnrealCV, Warning, TEXT("Level loaded"));
}


void AUnrealcvWorldController::Tick(float DeltaTime)
{

}
