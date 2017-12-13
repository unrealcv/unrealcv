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
	UPawnCamSensor* PawnCamSensor = NewObject<UPawnCamSensor>(Pawn, TEXT("Pawn Sensor")); // Make Pawn as the owner of the component
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

	// Update camera FOV
	// PlayerController->PlayerCameraManager->SetFOV(Config.FOV);

	// FObjectPainter::Get().Reset(GetPawn()->GetLevel());
	// FCaptureManager::Get().AttachGTCaptureComponentToCamera(GetPawn()); // TODO: Make this configurable in the editor

	// UpdateInput(Config.EnableInput);

	// FEngineShowFlags ShowFlags = World->GetGameViewport()->EngineShowFlags;
	// FPlayerViewMode::Get().SaveGameDefault(ShowFlags);

	// CurrentWorld = World;
}
