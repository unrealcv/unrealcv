#include "UnrealCVPrivate.h"
#include "ActorController.h"

FActorController::FActorController(AActor* InActor)
{
	Actor = InActor;
}

FRotator FActorController::GetRotation()
{
	return Actor->GetActorRotation();
}

FVector FActorController::GetLocation()
{
	return Actor->GetActorLocation();
}
