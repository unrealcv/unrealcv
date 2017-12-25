#include "UnrealCVPrivate.h"
#include "ActorController.h"

FActorController::FActorController(AActor* InActor)
{
	Actor = InActor;
}

FVector FActorController::GetLocation()
{
	return Actor->GetActorLocation();
}

bool FActorController::SetLocation(FVector Location)
{
	return Actor->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
}

FRotator FActorController::GetRotation()
{
	return Actor->GetActorRotation();
}

bool FActorController::SetRotation(FRotator Rotator)
{
	return Actor->SetActorRotation(Rotator);
}

EComponentMobility::Type FActorController::GetMobility()
{
	return Actor->GetRootComponent()->Mobility.GetValue();
}

void FActorController::Show()
{
	Actor->SetActorHiddenInGame(false);
}

void FActorController::Hide()
{
	Actor->SetActorHiddenInGame(true);
}

bool FActorController::GetAnnotationColor(FColor& AnnotationColor)
{
	FObjectAnnotator& Annotator = FUE4CVServer::Get().WorldController->ObjectAnnotator;
	return Annotator.GetAnnotationColor(Actor, AnnotationColor);
}

bool FActorController::SetAnnotationColor(const FColor& AnnotationColor)
{
	FObjectAnnotator& Annotator = FUE4CVServer::Get().WorldController->ObjectAnnotator;
	return Annotator.SetAnnotationColor(Actor, AnnotationColor);
}
