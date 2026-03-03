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
	return Actor.IsValid() ? Actor->GetActorLocation() : FVector::ZeroVector;
}

void FActorController::SetLocation(FVector Location)
{
	if (Actor.IsValid())
	{
		Actor->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

FRotator FActorController::GetRotation()
{
	return Actor.IsValid() ? Actor->GetActorRotation() : FRotator::ZeroRotator;
}

void FActorController::SetRotation(FRotator Rotator)
{
	if (Actor.IsValid())
	{
		Actor->SetActorRotation(Rotator);
	}
}

EComponentMobility::Type FActorController::GetMobility()
{
	if (!Actor.IsValid() || !Actor->GetRootComponent())
	{
		return EComponentMobility::Type::Movable;
	}
	return Actor->GetRootComponent()->Mobility.GetValue();
}

void FActorController::Show()
{
	if (Actor.IsValid())
	{
		Actor->SetActorHiddenInGame(false);
	}
}

void FActorController::Hide()
{
	if (Actor.IsValid())
	{
		Actor->SetActorHiddenInGame(true);
	}
}

void FActorController::GetAnnotationColor(FColor& AnnotationColor)
{
	if (!Actor.IsValid() || !FUnrealcvServer::Get().WorldController.IsValid())
	{
		return;
	}
	FObjectAnnotator& Annotator = FUnrealcvServer::Get().WorldController->ObjectAnnotator;
	Annotator.GetAnnotationColor(Actor.Get(), AnnotationColor);
}

void FActorController::SetAnnotationColor(const FColor& AnnotationColor)
{
	if (!Actor.IsValid() || !FUnrealcvServer::Get().WorldController.IsValid())
	{
		return;
	}
	FObjectAnnotator& Annotator = FUnrealcvServer::Get().WorldController->ObjectAnnotator;
	Annotator.SetAnnotationColor(Actor.Get(), AnnotationColor);
}
