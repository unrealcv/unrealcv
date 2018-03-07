// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "UE4CVWorldController.h"
#include "PawnCamSensor.h"
#include "VisionBP.h"
#include "UE4CVServer.h"

AUE4CVWorldController::AUE4CVWorldController(const FObjectInitializer& ObjectInitializer)
{

}

void AttachPawnSensor(APawn* Pawn)
{
	if (!IsValid(Pawn))
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("The pawn is invalid, can not attach a PawnSensor to it."));
		return;
	}
	ScreenLog("Attach a UE4CVSensor to the pawn");
	// Make sure this is the first one.
	UPawnCamSensor* PawnCamSensor = NewObject<UPawnCamSensor>(Pawn, TEXT("PawnSensor")); // Make Pawn as the owner of the component
	// UFusionCamSensor* FusionCamSensor = ConstructObject<UFusionCamSensor>(UFusionCamSensor::StaticClass(), Pawn);

	UWorld *PawnWorld = Pawn->GetWorld(), *GameWorld = FUE4CVServer::Get().GetGameWorld();
	// check(Pawn->GetWorld() == FUE4CVServer::GetGameWorld());
	check(PawnWorld == GameWorld);
	PawnCamSensor->AttachToComponent(Pawn->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	// AActor* OwnerActor = FusionCamSensor->GetOwner();
	PawnCamSensor->RegisterComponent(); // Is this neccessary?
}



void AUE4CVWorldController::BeginPlay()
{
	ScreenLog("Overwrite the world setting with some UnrealCV extensions");

	FUE4CVServer& UE4CVServer = FUE4CVServer::Get();
	if (UE4CVServer.NetworkManager && !UE4CVServer.NetworkManager->IsListening())
	{
		ScreenLog("The tcp server is not running");
	}

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	check(PlayerController);
	APawn* Pawn = PlayerController->GetPawn();
	FUE4CVServer& Server = FUE4CVServer::Get();
	if (IsValid(Pawn))
	{
		AttachPawnSensor(Pawn);
		UVisionBP::UpdateInput(Pawn, Server.Config.EnableInput);
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("No available pawn to mount a PawnSensor"));
	}

	ObjectAnnotator.AnnotateWorld(GetWorld());

	FEngineShowFlags ShowFlags = GetWorld()->GetGameViewport()->EngineShowFlags;
	FPlayerViewMode::Get().SaveGameDefault(ShowFlags);

	// TODO: remove legacy code
	// Update camera FOV

	//PlayerController->PlayerCameraManager->SetFOV(Server.Config.FOV);
	//FCaptureManager::Get().AttachGTCaptureComponentToCamera(Pawn);
}



void AUE4CVWorldController::OpenLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::OpenLevel(World, LevelName);
	UGameplayStatics::FlushLevelStreaming(World);
	UE_LOG(LogUnrealCV, Warning, TEXT("Level loaded"));
}
