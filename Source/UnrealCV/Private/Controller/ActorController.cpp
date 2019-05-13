#include "ActorController.h"
#include "ObjectAnnotator.h"
#include "UnrealcvServer.h"
#include "WorldController.h"

FActorController::FActorController(AActor* InActor)
{
	Actor = InActor;
}

FVector FActorController::GetLocation()
{
	return Actor->GetActorLocation();
}

void FActorController::SetLocation(FVector Location)
{
	Actor->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
}

FRotator FActorController::GetRotation()
{
	return Actor->GetActorRotation();
}

void FActorController::SetRotation(FRotator Rotator)
{
	Actor->SetActorRotation(Rotator);
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

void FActorController::GetAnnotationColor(FColor& AnnotationColor)
{
	FObjectAnnotator& Annotator = FUnrealcvServer::Get().WorldController->ObjectAnnotator;
	Annotator.GetAnnotationColor(Actor, AnnotationColor);
}

void FActorController::SetAnnotationColor(const FColor& AnnotationColor)
{
	FObjectAnnotator& Annotator = FUnrealcvServer::Get().WorldController->ObjectAnnotator;
	Annotator.SetAnnotationColor(Actor, AnnotationColor);
}
