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
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor)) { return FVector::ZeroVector; }
	return ResolvedActor->GetActorLocation();
}

void FActorController::SetLocation(FVector Location)
{
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor)) { return; }
	ResolvedActor->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
}

FRotator FActorController::GetRotation()
{
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor)) { return FRotator::ZeroRotator; }
	return ResolvedActor->GetActorRotation();
}

void FActorController::SetRotation(FRotator Rotator)
{
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor)) { return; }
	ResolvedActor->SetActorRotation(Rotator);
}

EComponentMobility::Type FActorController::GetMobility()
{
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor) || !IsValid(ResolvedActor->GetRootComponent()))
	{
		return EComponentMobility::Static;
	}
	return ResolvedActor->GetRootComponent()->Mobility.GetValue();
}

void FActorController::Show()
{
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor)) { return; }
	ResolvedActor->SetActorHiddenInGame(false);
}

void FActorController::Hide()
{
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor)) { return; }
	ResolvedActor->SetActorHiddenInGame(true);
}

void FActorController::GetAnnotationColor(FColor& AnnotationColor)
{
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor)) { return; }
	const auto WorldCtrl = FUnrealcvServer::Get().GetWorldController();
	if (!WorldCtrl.IsValid()) { return; }
	FObjectAnnotator& Annotator = WorldCtrl->ObjectAnnotator;
	Annotator.GetAnnotationColor(ResolvedActor, AnnotationColor);
}

void FActorController::SetAnnotationColor(const FColor& AnnotationColor)
{
	AActor* ResolvedActor = Actor.Get();
	if (!IsValid(ResolvedActor)) { return; }
	const auto WorldCtrl = FUnrealcvServer::Get().GetWorldController();
	if (!WorldCtrl.IsValid()) { return; }
	FObjectAnnotator& Annotator = WorldCtrl->ObjectAnnotator;
	Annotator.SetAnnotationColor(ResolvedActor, AnnotationColor);
}
