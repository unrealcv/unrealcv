// Weichao Qiu @ 2017
#include "UnrealCVPrivate.h"
#include "UE4CVWorldController.h"
#include "PawnCamSensor.h"

AUE4CVWorldController::AUE4CVWorldController(const FObjectInitializer& ObjectInitializer)
{

}

void AttachFusionSensorToPawn(APawn* Pawn)
{
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

	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	check(PlayerController);
	APawn* Pawn = PlayerController->GetPawn();

	AttachFusionSensorToPawn(Pawn);

	ObjectAnnotator.AnnotateStaticMesh();

	FEngineShowFlags ShowFlags = GetWorld()->GetGameViewport()->EngineShowFlags;
	FPlayerViewMode::Get().SaveGameDefault(ShowFlags);

	FUE4CVServer& Server = FUE4CVServer::Get();

	UpdateInput(Server.Config.EnableInput);

	// TODO: remove legacy code
	// Update camera FOV
	PlayerController->PlayerCameraManager->SetFOV(Server.Config.FOV);
	FCaptureManager::Get().AttachGTCaptureComponentToCamera(Pawn);
}


void AUE4CVWorldController::UpdateInput(bool Enable)
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PlayerController = World->GetFirstPlayerController();
	check(PlayerController);
	if (Enable)
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Enabling input"));
		PlayerController->GetPawn()->EnableInput(PlayerController);
	}
	else
	{
		UE_LOG(LogUnrealCV, Warning, TEXT("Disabling input"));
		PlayerController->GetPawn()->DisableInput(PlayerController);
	}
}

void AUE4CVWorldController::OpenLevel(FName LevelName)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UGameplayStatics::OpenLevel(World, LevelName);
	UGameplayStatics::FlushLevelStreaming(World);
	UE_LOG(LogUnrealCV, Warning, TEXT("Level loaded"));
}
